#ifndef SSL_AST_HPP
#define SSL_AST_HPP

#include "error.hpp"
#include "tokens.hpp"
#include <cstdint>
#include <vector>

enum class StmtType: char {
   varDecl, fnDecl, lambda, enumDecl, structDecl,
   ifStmt, ifClause, matchStmt, matchCase,
   forLoop, loop, whileLoop, doWhileLoop, breakStmt, continueStmt,
   returnStmt, unlessStmt, doStmt, importStmt,
   assignment, binary, unary, property, call,
   enumEntry, identifier, number, string, array, null, program,
};

constexpr const char *statementTypeStrings[] {
   "Variable Declaration", "Function Declaration", "Lambda", "Enumeration Declaration",
   "Structure Declaration", "If Statement", "If Clause", "Match Statement", "Match Case",
   "For Loop", "Loop", "While Loop", "Do While Loop", "Break Statement", "Continue Statement",
   "Return Statement", "Unless Statement", "Do Statement", "Import Statement", "Assignment",
   "Binary Expression", "Unary Expression", "Property Access", "Function Call", "Identifier",
   "Enumeration Entry", "Number", "String", "Array", "Null", "Scope",
};

constexpr const char *getStatementTypeAsString(StmtType type) {
   return statementTypeStrings[(size_t)type];
}

using NodeId = uint32_t;
using StringId = uint32_t;

struct NodeList {
   size_t start = 0;
   size_t size = 0;
};

struct VarDecl {
   bool isPublic;
   StringId identifier;
   NodeId value;
};

struct FnDecl {
   bool isPublic;
   StringId identifier;
   NodeId body;
   NodeList parameters;
};

struct Lambda {
   NodeId body;
   NodeList parameters;
};

struct EnumDecl {
   bool isPublic;
   StringId identifier;
   NodeList entries;
};

struct StructDecl {
   bool isPublic;
   StringId identifier;
   NodeList varDeclarations;
};

struct IfStmt {
   NodeId ifClause;
   NodeList elifClauses;
   NodeId elseClause;
};

struct IfClause {
   KeywordType keyword;
   NodeId expression;
   NodeId statement;
};

struct MatchStmt {
   NodeList cases;
   NodeId elseClause;
};

struct MatchCase {
   NodeId expression;
   NodeId statement;
};

struct ForLoop {
   StringId identifier;
   NodeId inExpression;
   NodeId body;
};

struct Loop {
   NodeId body;
};

struct WhileLoop {
   NodeId expression;
   NodeId statement;
};

struct DoWhileLoop {
   NodeId expression;
   NodeId statement;
};

struct BreakStmt {};

struct ContinueStmt {};

struct ReturnStmt {
   NodeId value;
};

struct UnlessStmt {
   NodeId statement;
   NodeId expression;
};

struct DoStmt {
   NodeId statement;
};

struct ImportStmt {
   NodeList values;
   StringId file;
   NodeId as;
};

struct Assignment {
   NodeId left, right;
   TokenType op;
};

struct BinaryExpr {
   NodeId left, right;
   TokenType op;
};

struct UnaryExpr {
   NodeId value;
   TokenType op;
};

struct PropertyAccess {
   NodeId left;
   NodeList right;
};

struct FunctionCall {
   StringId identifier;
   NodeList args;
};

struct EnumEntry {
   StringId identifier;
   NodeId value;
   NodeList args;
   NodeList argValues;
};

struct IdentifierLiteral {
   StringId id;
};

struct NumberLiteral {
   long double number;
};

struct StringLiteral {
   StringId id;
};

struct ArrayLiteral {
   NodeList values;
};

struct NullStmt {};

struct Program {
   NodeList nodes;
};

// Node logic

struct Node {
   StmtType type;
   size_t line;

   union {
      VarDecl varDecl;
      FnDecl fnDecl;
      Lambda lambda;
      EnumDecl enumDecl;
      StructDecl structDecl;
      IfStmt ifStmt;
      IfClause ifClause;
      MatchStmt matchStmt;
      MatchCase matchCase;
      ForLoop forLoop;
      Loop loop;
      WhileLoop whileLoop;
      DoWhileLoop doWhileLoop;
      BreakStmt breakStmt;
      ContinueStmt continueStmt;
      ReturnStmt returnStmt;
      UnlessStmt unlessStmt;
      DoStmt doStmt;
      ImportStmt importStmt;
      Assignment assignment;
      BinaryExpr binary;
      UnaryExpr unary;
      PropertyAccess property;
      FunctionCall fnCall;
      EnumEntry enumEntry;
      IdentifierLiteral identifier;
      NumberLiteral number;
      StringLiteral string;
      ArrayLiteral array;
      NullStmt null;
      Program program;
   };
};

struct ASTArena {
   ASTArena();

   StringId pushString(const std::string &string);
   std::string_view getString(StringId id) const;
   NodeList pushVector(const std::vector<NodeId> &values);

   Node &get(NodeId id);
   const Node &get(NodeId id) const;

   void printList(NodeList list, int indentation = 0) const;
   void print(NodeId id, int indentation = 0) const;

   template<StmtType type, class T>
   T &as(NodeId id) {
      Node &node = get(id);
      if (node.type != type) {
         raiseError(node.line, "Node type does not match desired type (%s != %s).",
            getStatementTypeAsString(node.type), getStatementTypeAsString(type));
      }

      switch (type) {
      case StmtType::varDecl:      return node.varDecl;
      case StmtType::fnDecl:       return node.fnDecl;
      case StmtType::lambda:       return node.lambda;
      case StmtType::enumDecl:     return node.enumDecl;
      case StmtType::structDecl:   return node.structDecl;
      case StmtType::ifStmt:       return node.ifStmt;
      case StmtType::ifClause:     return node.ifClause;
      case StmtType::matchStmt:    return node.matchStmt;
      case StmtType::matchCase:    return node.matchCase;
      case StmtType::forLoop:      return node.forLoop;
      case StmtType::loop:         return node.loop;
      case StmtType::whileLoop:    return node.whileLoop;
      case StmtType::doWhileLoop:  return node.doWhileLoop;
      case StmtType::breakStmt:    return node.breakStmt;
      case StmtType::continueStmt: return node.continueStmt;
      case StmtType::returnStmt:   return node.returnStmt;
      case StmtType::unlessStmt:   return node.unlessStmt;
      case StmtType::doStmt:       return node.doStmt;
      case StmtType::importStmt:   return node.importStmt;
      case StmtType::assignment:   return node.assignment;
      case StmtType::binary:       return node.binary;
      case StmtType::unary:        return node.unary;
      case StmtType::property:     return node.property;
      case StmtType::call:         return node.fnCall;
      case StmtType::enumEntry:    return node.enumEntry;
      case StmtType::identifier:   return node.identifier;
      case StmtType::number:       return node.number;
      case StmtType::string:       return node.string;
      case StmtType::array:        return node.array;
      case StmtType::null:         return node.null;
      case StmtType::program:      return node.program;
      }
   }

   // builders

   NodeId allocate(Node node);
   NodeId allocateVarDecl(bool isPublic, const std::string &identifier, NodeId value, size_t line);
   NodeId allocateFnDecl(bool isPublic, const std::string &identifier, NodeId body, const std::vector<NodeId> &params, size_t line);
   NodeId allocateLambda(NodeId body, const std::vector<NodeId> &params, size_t line);
   NodeId allocateEnumDecl(bool isPublic, const std::string &identifier, const std::vector<NodeId> &entries, size_t line);
   NodeId allocateStructDecl(bool isPublic, const std::string &identifier, const std::vector<NodeId> &decls, size_t line);
   NodeId allocateIfStmt(NodeId ifClause, const std::vector<NodeId> &elifClauses, NodeId elseClause, size_t line);
   NodeId allocateIfClause(KeywordType keyword, NodeId expression, NodeId statement, size_t line);
   NodeId allocateMatchStmt(const std::vector<NodeId> &cases, NodeId elseClause, size_t line);
   NodeId allocateMatchCase(NodeId expression, NodeId statement, size_t line);
   NodeId allocateForLoop(const std::string &identifier, NodeId inExpression, NodeId body, size_t line);
   NodeId allocateLoop(NodeId body, size_t line);
   NodeId allocateWhileLoop(NodeId expression, NodeId statement, size_t line);
   NodeId allocateDoWhileLoop(NodeId expression, NodeId statement, size_t line);
   NodeId allocateBreakStmt(size_t line);
   NodeId allocateContinueStmt(size_t line);
   NodeId allocateReturnStmt(NodeId value, size_t line);
   NodeId allocateUnlessStmt(NodeId statement, NodeId expression, size_t line);
   NodeId allocateDoStmt(NodeId statement, size_t line);
   NodeId allocateImportStmt(const std::vector<NodeId> &values, const std::string &file, NodeId as, size_t line);
   NodeId allocateAssignment(NodeId left, NodeId right, TokenType op, size_t line);
   NodeId allocateBinary(NodeId left, NodeId right, TokenType op, size_t line);
   NodeId allocateUnary(NodeId value, TokenType op, size_t line);
   NodeId allocatePropertyAccess(NodeId left, const std::vector<NodeId> &right, size_t line);
   NodeId allocateFnCall(const std::string &identifier, const std::vector<NodeId> &args, size_t line);
   NodeId allocateEnumEntry(const std::string &identifier, NodeId value, const std::vector<NodeId> &args, const std::vector<NodeId> &argValues, size_t line);
   NodeId allocateIdentifier(const std::string &string, size_t line);
   NodeId allocateNumber(long double number, size_t line);
   NodeId allocateString(const std::string &string, size_t line);
   NodeId allocateArray(const std::vector<NodeId> &values, size_t line);
   NodeId allocateNull(size_t line);
   NodeId allocateProgram(const std::vector<NodeId> &nodes, size_t line);

   // members

   std::vector<Node> nodes;
   std::vector<NodeId> children;
   std::vector<std::string> strings;
};

#endif
