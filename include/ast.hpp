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

// struct VarDecl {
//    bool isConstant, isPublic;
//    Node identifier, value;
// };

// struct FnDecl {
//    bool isPublic;
//    Node identifier, body;
//    std::vector<Node> parameters;
// };

// struct Lambda {
//    Node body;
//    std::vector<Node> parameters;
// };

// struct EnumDecl {
//    bool isPublic;
//    Node identifier;
//    std::vector<Node> entries;
// };

struct BinaryExpr {
   NodeId left, right;
   TokenType op;
};

struct NumberLiteral {
   long double number;
};

struct StringLiteral {
   StringId id;
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
      BinaryExpr binary;
      NumberLiteral number;
      StringLiteral string;
      NullStmt null;
      Program program;
   };
};

struct ASTArena {
   ASTArena();

   StringId pushString(const std::string &string);
   std::string_view getString(StringId id) const;

   Node &get(NodeId id);
   const Node &get(NodeId id) const;

   template<StmtType type, class T>
   T &as(NodeId id) {
      Node &node = get(id);
      if (node.type != type) {
         raiseError(node.line, "Node type does not match desired type (%s != %s).",
            getStatementTypeAsString(node.type), getStatementTypeAsString(type));
      }

      if constexpr (type == StmtType::binary)  return node.binary;
      if constexpr (type == StmtType::number)  return node.number;
      if constexpr (type == StmtType::string)  return node.string;
      if constexpr (type == StmtType::null)    return node.null;
      if constexpr (type == StmtType::program) return node.program;
   }

   // builders

   NodeId allocate(Node node);
   NodeId allocateBinary(NodeId left, NodeId right, TokenType op, size_t line);
   NodeId allocateNumber(long double number, size_t line);
   NodeId allocateString(const std::string &string, size_t line);
   NodeId allocateNull(size_t line);
   NodeId allocateProgram(const std::vector<NodeId> &nodes, size_t line);

   // members

   std::vector<Node> nodes;
   std::vector<NodeId> children;
   std::vector<std::string> strings;
};

#endif
