#ifndef SSL_VALUES_HPP
#define SSL_VALUES_HPP

#include "ast.hpp"

enum class ValueType: char {
   null, number, boolean, string, function, array
};

constexpr const char *valueTypeStrings[] {
   "Null", "Number", "Boolean", "String", "Function", "Array"
};

constexpr const char *getValueTypeAsString(ValueType type) {
   return valueTypeStrings[(size_t)type];
}

struct Interpreter;
using ValueId = uint32_t;
using ArrayId = uint32_t;

struct FunctionValue {
   NodeId function;
   struct Environment *env;
};

struct Value {
   ValueId negate(Interpreter &interpreter) const;
   ValueId add(Interpreter &interpreter, const Value &right) const;
   ValueId subtract(Interpreter &interpreter, const Value &right) const;
   ValueId multiply(Interpreter &interpreter, const Value &right) const;
   ValueId divide(Interpreter &interpreter, const Value &right) const;
   ValueId remainder(Interpreter &interpreter, const Value &right) const;
   ValueId pow(Interpreter &interpreter, const Value &right) const;

   bool equal(Interpreter &interpreter, const Value &right) const;
   bool greater(Interpreter &interpreter, const Value &right) const;

   std::string asString(Interpreter &interpreter) const;
   long double asNumber(Interpreter &interpreter) const;
   bool asBoolean(Interpreter &interpreter) const;

   // members
   
   ValueType type;
   size_t line;
   bool constant = false;

   union {
      long double number;
      bool boolean;
      StringId string;
      FunctionValue function;
      ArrayId array;
   };
};


#endif
