#ifndef SSL_PARSER_HPP
#define SSL_PARSER_HPP

#include "ast.hpp"

struct Parser {
   Parser(std::vector<Token> &tokens);
   Program &parse();

   // parsing

   NodeId parseStmt();
   NodeId parseVarDecl(bool isPublic);
   NodeId parseFnDecl(bool isPublic);
   NodeId parseEnumDecl(bool isPublic);
   NodeId parseStructDecl(bool isPublic);
   NodeId parseIfStmt();
   NodeId parseIfClause();
   NodeId parseMatchStmt();
   NodeId parseForLoop();
   NodeId parseLoop();
   NodeId parseWhileLoop();
   NodeId parseDoWhileLoopOrNewScope();
   NodeId parseBreak();
   NodeId parseContinue();
   NodeId parseReturn();
   NodeId parseUnless(NodeId statement);
   NodeId parseImport();

   NodeId parseRangeExpr();
   NodeId parseExpr();
   NodeId parseAssignmentExpr();
   NodeId parseLogicalOrExpr();
   NodeId parseLogicalAndExpr();
   NodeId parseEqualityExpr();
   NodeId parseRelationalExpr();
   NodeId parseAdditiveExpr();
   NodeId parseMultiplicativeExpr();
   NodeId parseExponentiativeExpr();
   NodeId parseUnaryExpr();
   NodeId parsePostfixExpr();
   NodeId parsePrimaryExpr();

   // utility

   void advance();
   void expect(StmtType type, TokenType expected);
   bool is(TokenType type) const;
   bool peek(TokenType type) const;
   Token &current();
   size_t line();

   // members

   std::vector<Token> &tokens;
   ASTArena arena;
   Program program;
   size_t index = 0;
};

#endif
