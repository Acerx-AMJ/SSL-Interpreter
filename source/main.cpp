#include "error.hpp"
#include "input.hpp"
#include "lexer.hpp"

int main(int argc, char *argv[]) {
   if (argc != 2) {
      raiseErrorNoLine("Expected exactly 2 arguments, got %d instead.", argc);
   }

   std::string input = argv[1];
   getInputOrReadFile(input);

   Lexer lexer (input);
   std::vector<Token> &tokens = lexer.lex();

   for (Token &token: tokens) {
      printf("%s - %s - %s\n", getTokenTypeAsString(token.type), getKeywordTypeAsString(token.keywordType), token.lexeme.c_str());
   }
   return 0;
}
