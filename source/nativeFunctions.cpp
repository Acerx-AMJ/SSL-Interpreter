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

std::string ntfFormatBase(const std::vector<ValueId> &args, Interpreter &interpreter, size_t line, const char *fname) {
   if (args.size() < 1) {
      raiseError(line, "%s: expected at least 1 argument.", fname);
   }

   Value &stringValue = interpreter.valuePool[args[0]];
   if (stringValue.type != ValueType::string) {
      raiseError(line, "%s: expected first argument to be a string but it is %s instead., fname",
         getValueTypeAsString(stringValue.type));
   }

   std::string result = interpreter.arena.strings[stringValue.string];
   size_t position = 0;

   for (size_t i = 1; i < args.size(); ++i) {
      Value &arg = interpreter.valuePool[args[i]];
      position = result.find("{}", position);

      if (position != std::string::npos) {
         result.replace(position, 2, arg.asPrintableString(interpreter));
      }
   }

   if (position == std::string::npos || result.find("{}") != std::string::npos) {
      raiseError(line, "%s: brace count does not match argument count.", fname);
   }
   return result;
}

ValueId ntfPrintf(const std::vector<ValueId> &args, Interpreter &interpreter, size_t line) {
   printf("%s", ntfFormatBase(args, interpreter, line, "printf").c_str());
   return null;
}

ValueId ntfPrintfln(const std::vector<ValueId> &args, Interpreter &interpreter, size_t line) {
   printf("%s\n", ntfFormatBase(args, interpreter, line, "printfln").c_str());
   return null;
}

ValueId ntfFormat(const std::vector<ValueId> &args, Interpreter &interpreter, size_t line) {
   std::string format = ntfFormatBase(args, interpreter, line, "format");
   return interpreter.allocateString(format, line);
}

// native map

static const inline std::unordered_map<std::string_view, NtFunc> nativeFunctions {
   {"print", ntfPrint}, {"println", ntfPrintln}, {"printf", ntfPrintf}, {"printfln", ntfPrintfln},
   {"format", ntfFormat}
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
