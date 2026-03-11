#ifndef SSL_LEXER_HPP
#define SSL_LEXER_HPP

#include "tokens.hpp"
#include <vector>

struct Lexer {
   Lexer(const std::string &code);
   std::vector<Token> &lex();

   // helper functions

   char current();
   char peek();
   char advance();
   char getEscapeCode(char escape);

   void pushToken(TokenType type, const std::string &lexeme);
   void pushTokenKeyword(KeywordType type, const std::string &lexeme);

   // members

   const std::string &code;
   std::vector<Token> tokens;
   size_t index = 0;
   size_t line = 1;
};

#endif
