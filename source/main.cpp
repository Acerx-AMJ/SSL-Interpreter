#include "compiler.hpp"
#include "error.hpp"
#include "input.hpp"
#include "lexer.hpp"
#include "parser.hpp"
#include "vm.hpp"

int main(int argc, char *argv[]) {
   if (argc < 2) {
      raiseErrorNoLine("Expected at least 2 arguments, got %d instead.", argc);
   }

   std::string input = argv[1];
   getInputOrReadFile(input);
   setProgramCode(&input);

   Lexer lexer (input);
   std::vector<Token> &tokens = lexer.lex();
   // for (Token &token: tokens) {
   //    printf("%zu: %s Type: %s\n", token.line, token.lexeme.c_str(), getTokenTypeAsString(token.type));
   // }

   Parser parser (tokens);
   Program &program = parser.parse();
   // parser.arena.printList(program.nodes);

   Compiler compiler (parser.arena);
   Chunk chunk = compiler.compile(program.nodes);

   VM interpreter;
   interpreter.execute(chunk);
   return 0;
}
