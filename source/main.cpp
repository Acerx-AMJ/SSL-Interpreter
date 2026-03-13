#include "input.hpp"
#include "lexer.hpp"
#include "parser.hpp"

int main(int argc, char *argv[]) {
   if (argc != 2) {
      raiseErrorNoLine("Expected 2 arguments, got %d instead.", argc);
   }

   std::string input = argv[1];
   getInputOrReadFile(input);
   setProgramCode(&input);

   Lexer lexer (input);
   std::vector<Token> &tokens = lexer.lex();

   Parser parser (tokens);
   Program &program = parser.parse();
   parser.arena.printList(program.nodes);
   return 0;
}
