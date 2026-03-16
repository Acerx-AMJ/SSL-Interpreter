#ifndef SSL_VALUES_HPP
#define SSL_VALUES_HPP

#include "ast.hpp"

enum class ValueType: char {
   null, number, boolean, string, function
};

constexpr const char *valueTypeStrings[] {
   "Null", "Number", "Boolean", "String", "Function"
};

constexpr const char *getValueTypeAsString(ValueType type) {
   return valueTypeStrings[(size_t)type];
}

struct Interpreter;
using ValueId = uint32_t;

struct Value {
   ValueId negate(Interpreter &interpreter) const;
   ValueId add(Interpreter &interpreter, const Value &right) const;
   ValueId subtract(Interpreter &interpreter, const Value &right) const;
   ValueId multiply(Interpreter &interpreter, const Value &right) const;
   ValueId divide(Interpreter &interpreter, const Value &right) const;
   ValueId remainder(Interpreter &interpreter, const Value &right) const;
   ValueId pow(Interpreter &interpreter, const Value &right) const;
   ValueId equal(Interpreter &interpreter, const Value &right) const;
   ValueId notEqual(Interpreter &interpreter, const Value &right) const;
   ValueId greater(Interpreter &interpreter, const Value &right) const;
   ValueId greaterEqual(Interpreter &interpreter, const Value &right) const;
   ValueId smaller(Interpreter &interpreter, const Value &right) const;
   ValueId smallerEqual(Interpreter &interpreter, const Value &right) const;

   std::string asString(Interpreter &interpreter) const;
   long double asNumber(Interpreter &interpreter) const;
   bool asBoolean(Interpreter &interpreter) const;

   // members
   
   ValueType type;
   size_t line;

   union {
      long double number;
      bool boolean;
      StringId string;
      NodeId function;
   };
};


#endif
