#ifndef SSL_VALUES_HPP
#define SSL_VALUES_HPP

#include "ast.hpp"

enum class ValueType: char {
   null, number, boolean, character, string, function, ntFunction, array
};

constexpr const char *valueTypeStrings[] {
   "Null", "Number", "Boolean", "Character", "String", "Function", "Native Function", "Array"
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

struct CharacterValue {
   StringId stringId;
   size_t index;
};

struct Value {
   std::string asPrintable(Interpreter &interpreter);

   ValueId negate(Interpreter &interpreter) const;
   ValueId add(Interpreter &interpreter, const Value &right) const;
   ValueId subtract(Interpreter &interpreter, const Value &right) const;
   ValueId multiply(Interpreter &interpreter, const Value &right) const;
   ValueId divide(Interpreter &interpreter, const Value &right) const;
   ValueId remainder(Interpreter &interpreter, const Value &right) const;
   ValueId pow(Interpreter &interpreter, const Value &right) const;

   bool equal(Interpreter &interpreter, const Value &right) const;
   bool greater(Interpreter &interpreter, const Value &right) const;

   char asChar(Interpreter &interpreter) const;
   std::string asString(Interpreter &interpreter) const;
   long double asNumber(Interpreter &interpreter) const;
   bool asBoolean(Interpreter &interpreter) const;

   // members
   
   bool constant = false;
   bool lvalue = false;
   bool ref = false;
   ValueType type;
   size_t line;

   union {
      long double number;
      bool boolean;
      CharacterValue character;
      StringId string;
      FunctionValue function;
      StringId nativeFn;
      ArrayId array;
   };
};


#endif
