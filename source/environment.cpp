#include "environment.hpp"
#include "error.hpp"
#include "interpreter.hpp"

Environment::Environment(Environment *parent)
   : parent(parent) {}

Environment::Environment()
   : parent(nullptr) {

}

void Environment::declare(Interpreter &interpreter, bool isConstant, const std::string &identifier, ValueId value, size_t line) {
   if (variables.find(identifier) != variables.end()) {
      raiseError(line, "Cannot shadow variable in the same scope.");
   }
   variables[identifier] = value;
   interpreter.valuePool[value].constant = isConstant;
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
