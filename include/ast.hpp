#ifndef SSL_AST_HPP
#define SSL_AST_HPP

#include "tokens.hpp"
#include <cstdint>
#include <vector>

enum class StmtType: char {
   varDecl, fnDecl, lambda, enumDecl, structDecl,
   ifStmt, ifClause, matchStmt,
   forLoop, loop, whileLoop, doWhileLoop, breakStmt, continueStmt,
   returnStmt, unlessStmt, importStmt,
   assignment, binary, unary, property, arraySubscript, call, range,
   identifier, number, string, array, null, program,
};

constexpr const char *statementTypeStrings[] {
   "Variable Declaration", "Function Declaration", "Lambda", "Enumeration Declaration",
   "Structure Declaration", "If Statement", "If Clause", "Match Statement", "For Loop",
   "Loop", "While Loop", "Do While Loop", "Break Statement", "Continue Statement",
   "Return Statement", "Unless Statement", "Import Statement", "Assignment", "Binary Expression",
   "Unary Expression", "Property Access", "Array Subscript", "Function Call", "Range",
   "Identifier", "Number", "String", "Array", "Null", "Scope",
};

constexpr const char *getStatementTypeAsString(StmtType type) {
   return statementTypeStrings[(size_t)type];
}

using NodeId = uint32_t;
using StringId = uint32_t;

constexpr NodeId null = 0;

struct NodeList {
   size_t start;
   size_t size;
};

struct VarDecl {
   bool isPublic;
   bool isConstant;
   StringId identifier;
   NodeId value;
};

struct FnDecl {
   bool isPublic;
   bool returnsRef;
   StringId module;
   StringId identifier;
   NodeList body;
   NodeList parameters;
};

struct Lambda {
   bool returnsRef;
   NodeList body;
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
   TokenType keyword;
   NodeId expression;
   NodeList statement;
};

struct MatchStmt {
   NodeId expression;
   NodeList cases;
   NodeId elseClause;
};

struct ForLoop {
   bool reversed;
   StringId identifier;
   StringId indexIdentifier;
   NodeId inExpression;
   NodeList body;
};

struct Loop {
   NodeList body;
};

struct WhileLoop {
   NodeId expression;
   NodeList statement;
};

struct DoWhileLoop {
   NodeId expression;
   NodeList statement;
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

struct ImportStmt {
   NodeList values;
   StringId file;
   StringId as;
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
   bool nullChecked;
   NodeId left;
   StringId right;
};

struct ArraySubscript {
   NodeId left;
   NodeId expression;
};

struct FunctionCall {
   NodeId left;
   NodeList args;
};

struct Range {
   bool inclusive;
   NodeId left;
   NodeId right;
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
   bool ref = false;
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
      ForLoop forLoop;
      Loop loop;
      WhileLoop whileLoop;
      DoWhileLoop doWhileLoop;
      BreakStmt breakStmt;
      ContinueStmt continueStmt;
      ReturnStmt returnStmt;
      UnlessStmt unlessStmt;
      ImportStmt importStmt;
      Assignment assignment;
      BinaryExpr binary;
      UnaryExpr unary;
      PropertyAccess property;
      ArraySubscript arraySubscript;
      FunctionCall fnCall;
      Range range;
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

   // builders

   NodeId allocate(Node node);
   NodeId allocateVarDecl(bool isPublic, bool isConstant, const std::string &identifier, NodeId value, size_t line);
   NodeId allocateFnDecl(bool isPublic, bool returnsRef, const std::string &module, const std::string &identifier, const std::vector<NodeId> &body, const std::vector<NodeId> &params, size_t line);
   NodeId allocateLambda(bool returnsRef, const std::vector<NodeId> &body, const std::vector<NodeId> &params, size_t line);
   NodeId allocateEnumDecl(bool isPublic, const std::string &identifier, const std::vector<NodeId> &entries, size_t line);
   NodeId allocateStructDecl(bool isPublic, const std::string &identifier, const std::vector<NodeId> &decls, size_t line);
   NodeId allocateIfStmt(NodeId ifClause, const std::vector<NodeId> &elifClauses, NodeId elseClause, size_t line);
   NodeId allocateIfClause(TokenType keyword, NodeId expression, const std::vector<NodeId> &statement, size_t line);
   NodeId allocateMatchStmt(NodeId expression, const std::vector<NodeId> &cases, NodeId elseClause, size_t line);
   NodeId allocateForLoop(bool reversed, const std::string &identifier, const std::string &indexIdentifier, NodeId inExpression, const std::vector<NodeId> &body, size_t line);
   NodeId allocateLoop(const std::vector<NodeId> &body, size_t line);
   NodeId allocateWhileLoop(NodeId expression, const std::vector<NodeId> &statement, size_t line);
   NodeId allocateDoWhileLoop(NodeId expression, const std::vector<NodeId> &statement, size_t line);
   NodeId allocateBreakStmt(size_t line);
   NodeId allocateContinueStmt(size_t line);
   NodeId allocateReturnStmt(NodeId value, size_t line);
   NodeId allocateUnlessStmt(NodeId statement, NodeId expression, size_t line);
   NodeId allocateImportStmt(const std::vector<NodeId> &values, const std::string &file, const std::string &as, size_t line);
   NodeId allocateAssignment(NodeId left, NodeId right, TokenType op, size_t line);
   NodeId allocateBinary(NodeId left, NodeId right, TokenType op, size_t line);
   NodeId allocateUnary(NodeId value, TokenType op, size_t line);
   NodeId allocatePropertyAccess(bool nullChecked, NodeId left, const std::string &right, size_t line);
   NodeId allocateArraySubscript(NodeId left, NodeId expression, size_t line);
   NodeId allocateFnCall(NodeId left, const std::vector<NodeId> &args, size_t line);
   NodeId allocateRange(bool inclusive, NodeId left, NodeId right, size_t line);
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
