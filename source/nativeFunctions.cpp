#include "error.hpp"
#include "interpreter.hpp"
#include "nativeFunctions.hpp"

// print utility

ValueId ntfPrint(const std::vector<ValueId> &args, Interpreter &interpreter, size_t line) {
   for (ValueId id: args) {
      Value &value = interpreter.valuePool[id];
      value.print(interpreter);
      putchar(' ');
   }
   return null;
}

ValueId ntfPrintln(const std::vector<ValueId> &args, Interpreter &interpreter, size_t line) {
   ntfPrint(args, interpreter, line);
   putchar('\n');
   return null;
}

// native map

static const inline std::unordered_map<std::string_view, NtFunc> nativeFunctions {
   {"print", ntfPrint}, {"println", ntfPrintln}
};

bool isNativeFunction(const std::string &identifier) {
   return nativeFunctions.find(identifier) != nativeFunctions.end();
}

const NtFunc &getNativeFunction(const std::string &identifier, size_t line) {
   if (nativeFunctions.find(identifier) == nativeFunctions.end()) {
      raiseError(line, "Function '%s' does not exist.", identifier.c_str());
   }
   return nativeFunctions.at(identifier);
}
