#include "environment.hpp"
#include "error.hpp"
#include "interpreter.hpp"

Environment::Environment(Environment *parent)
   : parent(parent) {}

Environment::Environment(Interpreter &interpreter)
   : parent(nullptr) {
   
   // globals
   declare(interpreter, true, "null", null, 0);
   declare(interpreter, true, "true", interpreter.allocateBoolean(true, 0), 0);
   declare(interpreter, true, "false", interpreter.allocateBoolean(false, 0), 0);

   // // types
   // declare(interpreter, true, "number", interpreter.allocateString("number", 0), 0);
   // declare(interpreter, true, "boolean", interpreter.allocateString("boolean", 0), 0);
   // declare(interpreter, true, "string", interpreter.allocateString("string", 0), 0);
   // declare(interpreter, true, "function", interpreter.allocateString("function", 0), 0);
   // declare(interpreter, true, "array", interpreter.allocateString("array", 0), 0);

   // native functions
   declare(interpreter, true, "print", interpreter.allocateNtFunction(ntfprint), 0);
   declare(interpreter, true, "println", interpreter.allocateNtFunction(ntfprintln), 0);
   declare(interpreter, true, "printf", interpreter.allocateNtFunction(ntfprintf), 0);
   declare(interpreter, true, "printfln", interpreter.allocateNtFunction(ntfprintfln), 0);
   declare(interpreter, true, "format", interpreter.allocateNtFunction(ntfformat), 0);
}

void Environment::declare(Interpreter &interpreter, bool isConstant, const std::string &identifier, ValueId value, size_t line) {
   if (identifier == "_") {
      return;
   }
   
   if (variables.find(identifier) != variables.end()) {
      raiseError(line, "Cannot shadow variable in the same scope.");
   }
   variables[identifier] = value;
   interpreter.valuePool[value].constant = isConstant;
   interpreter.valuePool[value].lvalue = true;
}

bool Environment::exists(const std::string &identifier) {
   if (variables.find(identifier) != variables.end()) {
      return true;
   }
   return parent && parent->exists(identifier);
}

ValueId Environment::get(const std::string &identifier, size_t line) {
   Environment &env = resolve(identifier, line);
   return env.variables[identifier];
}

Environment &Environment::resolve(const std::string &identifier, size_t line) {
   if (variables.find(identifier) != variables.end()) {
      return *this;
   }

   if (!parent) {
      raiseError(line, "Variable '%s' does not exist in the given scope.", identifier.c_str());
   }
   return parent->resolve(identifier, line);
}
