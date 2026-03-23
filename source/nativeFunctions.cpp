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

// type utility

ValueId ntfTypeOf(const std::vector<ValueId> &args, Interpreter &interpreter, size_t line) {
   if (args.size() != 1) {
      raiseError(line, "typeOf: expected exactly 1 argument.");
   }
   
   switch (interpreter.valuePool[args[0]].type) {
   case ValueType::null:
      return null;
   case ValueType::number:
      return interpreter.allocateString("number", line);
   case ValueType::boolean:
      return interpreter.allocateString("boolean", line);
   case ValueType::string:
      return interpreter.allocateString("string", line);
   case ValueType::function:
   case ValueType::ntFunction:
      return interpreter.allocateString("function", line);
   case ValueType::array:
      return interpreter.allocateString("array", line);
   }
   return null;
}

ValueId ntfIsNull(const std::vector<ValueId> &args, Interpreter &interpreter, size_t line) {
   if (args.size() != 1) {
      raiseError(line, "isNull: expected exactly 1 argument.");
   }
   return interpreter.allocateBoolean(interpreter.valuePool[args[0]].type == ValueType::null, line);
}

ValueId ntfIsNumber(const std::vector<ValueId> &args, Interpreter &interpreter, size_t line) {
   if (args.size() != 1) {
      raiseError(line, "isNumber: expected exactly 1 argument.");
   }
   return interpreter.allocateBoolean(interpreter.valuePool[args[0]].type == ValueType::number, line);
}

ValueId ntfIsBoolean(const std::vector<ValueId> &args, Interpreter &interpreter, size_t line) {
   if (args.size() != 1) {
      raiseError(line, "isBoolean: expected exactly 1 argument.");
   }
   return interpreter.allocateBoolean(interpreter.valuePool[args[0]].type == ValueType::boolean, line);
}

ValueId ntfIsString(const std::vector<ValueId> &args, Interpreter &interpreter, size_t line) {
   if (args.size() != 1) {
      raiseError(line, "isString: expected exactly 1 argument.");
   }
   return interpreter.allocateBoolean(interpreter.valuePool[args[0]].type == ValueType::string, line);
}

ValueId ntfIsFunction(const std::vector<ValueId> &args, Interpreter &interpreter, size_t line) {
   if (args.size() != 1) {
      raiseError(line, "isFunction: expected exactly 1 argument.");
   }
   ValueType type = interpreter.valuePool[args[0]].type;
   return interpreter.allocateBoolean(type == ValueType::function || type == ValueType::ntFunction, line);
}

ValueId ntfIsArray(const std::vector<ValueId> &args, Interpreter &interpreter, size_t line) {
   if (args.size() != 1) {
      raiseError(line, "isArray: expected exactly 1 argument.");
   }
   return interpreter.allocateBoolean(interpreter.valuePool[args[0]].type == ValueType::array, line);
}

ValueId ntfToNumber(const std::vector<ValueId> &args, Interpreter &interpreter, size_t line) {
   if (args.size() != 1) {
      raiseError(line, "toNumber: expected exactly 1 argument.");
   }
   return interpreter.allocateNumber(interpreter.valuePool[args[0]].asNumber(interpreter), line);
}

ValueId ntfToBoolean(const std::vector<ValueId> &args, Interpreter &interpreter, size_t line) {
   if (args.size() != 1) {
      raiseError(line, "toBoolean: expected exactly 1 argument.");
   }
   return interpreter.allocateBoolean(interpreter.valuePool[args[0]].asBoolean(interpreter), line);
}

ValueId ntfToString(const std::vector<ValueId> &args, Interpreter &interpreter, size_t line) {
   if (args.size() != 1) {
      raiseError(line, "toString: expected exactly 1 argument.");
   }
   return interpreter.allocateString(interpreter.valuePool[args[0]].asString(interpreter), line);
}

// native map

static const inline std::unordered_map<std::string_view, NtFunc> nativeFunctions {
   {"print", ntfPrint}, {"println", ntfPrintln}, {"printf", ntfPrintf}, {"printfln", ntfPrintfln},
   {"format", ntfFormat}, {"typeof", ntfTypeOf}, {"isNull", ntfIsNull}, {"isNumber", ntfIsNumber},
   {"isBoolean", ntfIsBoolean}, {"isString", ntfIsString}, {"isFunction", ntfIsFunction},
   {"isArray", ntfIsArray}, {"toNumber", ntfToNumber}, {"toBoolean", ntfToBoolean},
   {"toString", ntfToString}
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
