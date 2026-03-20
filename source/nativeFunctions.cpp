#include "error.hpp"
#include "interpreter.hpp"
#include "nativeFunctions.hpp"
#include <fmt/args.h>

// print utility

ValueId ntfPrint(const std::vector<ValueId> &args, Interpreter &interpreter, size_t line) {
   for (ValueId id: args) {
      Value &value = interpreter.valuePool[id];
      fmt::print("{} ", value.asPrintable(interpreter));
   }
   return null;
}

ValueId ntfPrintln(const std::vector<ValueId> &args, Interpreter &interpreter, size_t line) {
   ntfPrint(args, interpreter, line);
   putchar('\n');
   return null;
}

std::string formatValues(const std::string &fmtStr, const std::vector<ValueId> &args, Interpreter &interpreter) {
   fmt::dynamic_format_arg_store<fmt::format_context> store;

   for (size_t i = 1; i < args.size(); ++i) {
      Value &value = interpreter.valuePool[args[i]];
      if (value.type == ValueType::number) {
         store.push_back(value.number);
      } else {
         store.push_back(value.asPrintable(interpreter));
      }
   }
   return fmt::vformat(fmtStr, store);
}

ValueId ntfPrintf(const std::vector<ValueId> &args, Interpreter &interpreter, size_t line) {
   if (args.size() < 1) {
      raiseError(line, "printf: expected at least 1 argument.");
   }

   Value &stringValue = interpreter.valuePool[args[0]];
   if (stringValue.type != ValueType::string) {
      raiseError(line, "printf: expected first argument to be a string but it is %s instead.",
         getValueTypeAsString(stringValue.type));
   }

   try {
      std::string result = interpreter.arena.strings[stringValue.string];
      printf("%s", formatValues(result, args, interpreter).c_str());
      return null;
   }
   catch (...) {
      raiseError(line, "printf: brace count does not match argument count/the formatting is invalid.");
   }
}

ValueId ntfPrintfln(const std::vector<ValueId> &args, Interpreter &interpreter, size_t line) {
   if (args.size() < 1) {
      raiseError(line, "printfln: expected at least 1 argument.");
   }

   Value &stringValue = interpreter.valuePool[args[0]];
   if (stringValue.type != ValueType::string) {
      raiseError(line, "printfln: expected first argument to be a string but it is %s instead.",
         getValueTypeAsString(stringValue.type));
   }

   try {
      std::string result = interpreter.arena.strings[stringValue.string];
      printf("%s\n", formatValues(result, args, interpreter).c_str());
      return null;
   }
   catch (...) {
      raiseError(line, "printfln: brace count does not match argument count/the formatting is invalid.");
   }
}

ValueId ntfFormat(const std::vector<ValueId> &args, Interpreter &interpreter, size_t line) {
   if (args.size() < 1) {
      raiseError(line, "format: expected at least 1 argument.");
   }

   Value &stringValue = interpreter.valuePool[args[0]];
   if (stringValue.type != ValueType::string) {
      raiseError(line, "format: expected first argument to be a string but it is %s instead.",
         getValueTypeAsString(stringValue.type));
   }

   try {
      std::string result = interpreter.arena.strings[stringValue.string];
      return interpreter.allocateString(formatValues(result, args, interpreter), line);
   }
   catch (...) {
      raiseError(line, "format: brace count does not match argument count/the formatting is invalid.");
   }
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
