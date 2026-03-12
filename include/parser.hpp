#ifndef SSL_PARSER_HPP
#define SSL_PARSER_HPP

#include "ast.hpp"

struct Parser {
   Parser(std::vector<Token> &tokens);
   Program &parse();

   // parsing

   NodeId parseStmt();

   NodeId parseExpr();
   NodeId parseAdditiveExpr();
   NodeId parseMultiplicativeExpr();
   NodeId parseExponentiativeExpr();
   NodeId parsePrimaryExpr();

   // utility

   void advance();
   bool is(TokenType type) const;
   Token &current();
   size_t line();

   // members

   std::vector<Token> &tokens;
   ASTArena arena;
   Program program;
   size_t index = 0;
};

#endif
