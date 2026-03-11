#include "error.hpp"
#include "input.hpp"
#include "lexer.hpp"

int main(int argc, char *argv[]) {
   if (argc != 2) {
      raiseErrorNoLine("Expected 2 arguments, got %d instead.", argc);
   }

   std::string input = argv[1];
   getInputOrReadFile(input);
   setProgramCode(&input);

   Lexer lexer (input);
   std::vector<Token> &tokens = lexer.lex();

   for (Token &token: tokens) {
      printf("%lu; %s - %s\n", token.line, getTokenTypeAsString(token.type), token.lexeme.c_str());
   }
   return 0;
}
