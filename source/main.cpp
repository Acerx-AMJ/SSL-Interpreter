// #include "error.hpp"
// #include "input.hpp"
// #include "interpreter.hpp"
// #include "lexer.hpp"
// #include "parser.hpp"

#include "chunk.hpp"
#include "vm.hpp"
int main(int argc, char *argv[]) {
   Chunk chunk;
   chunk.addConstant(50.0);
   chunk.addConstant(25.0);

   chunk.write(OP_CONSTANT, 0);
   chunk.write(0, 0);
   chunk.write(OP_CONSTANT, 1);
   chunk.write(1, 1);
   chunk.write(OP_ADD, 2);
   chunk.write(OP_RETURN, 3);

   chunk.disassemble();

   VM vm;
   vm.execute(chunk);

   // if (argc < 2) {
   //    raiseErrorNoLine("Expected at least 2 arguments, got %d instead.", argc);
   // }

   // std::string input = argv[1];
   // getInputOrReadFile(input);
   // setProgramCode(&input);

   // Lexer lexer (input);
   // std::vector<Token> &tokens = lexer.lex();
   // // for (Token &token: tokens) {
   // //    printf("%zu: %s Type: %s\n", token.line, token.lexeme.c_str(), getTokenTypeAsString(token.type));
   // // }

   // Parser parser (tokens);
   // Program &program = parser.parse();
   // // parser.arena.printList(program.nodes);

   // Interpreter interpreter (parser.arena);
   // Environment global (interpreter);

   // interpreter.evaluate(program.nodes, global);
   // interpreter.callMain(global);
   // return 0;
}
