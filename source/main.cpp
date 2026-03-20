#include "error.hpp"
#include "input.hpp"
#include "interpreter.hpp"
#include "lexer.hpp"
#include "parser.hpp"

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

   Interpreter interpreter (parser.arena);
   Environment global (interpreter);
   interpreter.evaluate(program.nodes, global);

   if (global.exists("main")) {
      ValueId main = global.get("main", 0);
      Value &m = interpreter.valuePool[main];

      if (m.type != ValueType::function) {
         raiseError(m.line, "Expected 'main' to be a Function but it is %d instead.",
            getValueTypeAsString(m.type));
      }

      Node &n = parser.arena.get(m.function.function);
      size_t argcount = (n.type == StmtType::lambda ? n.lambda.parameters.size : n.fnDecl.parameters.size);

      if (argcount > 1) {
         raiseError(m.line, "Expected 'main' to have 0 or 1 parameters, got %d instead.", argcount);
      }

      if (argcount == 1) {
         
      }
      else {
         Environment newEnvironment (&global);
         interpreter.evaluate(n.type == StmtType::lambda ? n.lambda.body : n.fnDecl.body, newEnvironment);
      }
   }
   return 0;
}
