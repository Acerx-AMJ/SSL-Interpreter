#include "error.hpp"
#include "interpreter.hpp"
#include "values.hpp"
#include <cmath>

ValueId Value::negate(Interpreter &interpreter) const {
   if (type == ValueType::null)   return null;
   if (type == ValueType::number) return interpreter.allocateNumber(-number, line);
   raiseError(line, "Cannot perform negation on %s value.", getValueTypeAsString(type));
}

ValueId Value::add(Interpreter &interpreter, const Value &r) const {
   if (type == ValueType::function || r.type == ValueType::function
    || type == ValueType::boolean  || r.type == ValueType::boolean) {
      raiseError(line, "Cannot perform addition on %s value and %s value.",
         getValueTypeAsString(type), getValueTypeAsString(r.type));
   }
   
   if (type == ValueType::null || r.type == ValueType::null) return null;
   if (type == ValueType::string || r.type == ValueType::string) {
      const std::string &first  = interpreter.arena.strings[string];
      const std::string &second = interpreter.arena.strings[r.string];
      return interpreter.allocateString(first + second, line);
   }
   return interpreter.allocateNumber(number + r.number, line);
}

ValueId Value::subtract(Interpreter &interpreter, const Value &r) const {
   if (type == ValueType::function || r.type == ValueType::function
    || type == ValueType::boolean  || r.type == ValueType::boolean
    || type == ValueType::string   || r.type == ValueType::string) {
      raiseError(line, "Cannot perform subtraction on %s value and %s value.",
         getValueTypeAsString(type), getValueTypeAsString(r.type));
   }

   if (type == ValueType::null || r.type == ValueType::null) return null;
   return interpreter.allocateNumber(number - r.number, line);
}

ValueId Value::multiply(Interpreter &interpreter, const Value &r) const {
   if (type == ValueType::function || r.type == ValueType::function
    || type == ValueType::boolean  || r.type == ValueType::boolean
    || type == ValueType::string   || r.type == ValueType::string) {
      raiseError(line, "Cannot perform multiplication on %s value and %s value.",
         getValueTypeAsString(type), getValueTypeAsString(r.type));
   }

   if (type == ValueType::null || r.type == ValueType::null) return null;
   return interpreter.allocateNumber(number * r.number, line);
}

ValueId Value::divide(Interpreter &interpreter, const Value &r) const {
   if (type == ValueType::function || r.type == ValueType::function
    || type == ValueType::boolean  || r.type == ValueType::boolean
    || type == ValueType::string   || r.type == ValueType::string) {
      raiseError(line, "Cannot perform division on %s value and %s value.",
         getValueTypeAsString(type), getValueTypeAsString(r.type));
   }

   if (type == ValueType::null || r.type == ValueType::null) return null;
   if (r.number == 0.0) return interpreter.allocateNumber(0.0, line); // defined behaviour
   return interpreter.allocateNumber(number / r.number, line);
}

ValueId Value::remainder(Interpreter &interpreter, const Value &r) const {
   if (type == ValueType::function || r.type == ValueType::function
    || type == ValueType::boolean  || r.type == ValueType::boolean
    || type == ValueType::string   || r.type == ValueType::string) {
      raiseError(line, "Cannot perform modulo on %s value and %s value.",
         getValueTypeAsString(type), getValueTypeAsString(r.type));
   }

   if (type == ValueType::null || r.type == ValueType::null) return null;
   if (r.number == 0.0) return interpreter.allocateNumber(0.0, line); // defined behaviour
   return interpreter.allocateNumber(fmodl(number, r.number), line);
}

ValueId Value::pow(Interpreter &interpreter, const Value &r) const {
   if (type == ValueType::function || r.type == ValueType::function
    || type == ValueType::boolean  || r.type == ValueType::boolean
    || type == ValueType::string   || r.type == ValueType::string) {
      raiseError(line, "Cannot perform exponentiation on %s value and %s value.",
         getValueTypeAsString(type), getValueTypeAsString(r.type));
   }

   if (type == ValueType::null || r.type == ValueType::null) return null;
   return interpreter.allocateNumber(powl(number, r.number), line);
}

ValueId Value::equal(Interpreter &interpreter, const Value &r) const {
   if (type != r.type)              return interpreter.allocateBoolean(false, line);
   if (type == ValueType::null)     return interpreter.allocateBoolean(true, line);
   if (type == ValueType::number)   return interpreter.allocateBoolean(number == r.number, line);
   if (type == ValueType::boolean)  return interpreter.allocateBoolean(boolean == r.boolean, line);
   if (type == ValueType::function) return interpreter.allocateBoolean(function.function == r.function.function, line);
   
   const std::string &first  = interpreter.arena.strings[string];
   const std::string &second = interpreter.arena.strings[r.string];
   return interpreter.allocateBoolean(first == second, line);
}

ValueId Value::notEqual(Interpreter &interpreter, const Value &r) const {
   ValueId isEqual = equal(interpreter, r);
   bool &result = interpreter.valuePool[isEqual].boolean;
   result = !result;
   return isEqual;
}

ValueId Value::greater(Interpreter &interpreter, const Value &r) const {
   if (type == ValueType::number && r.type == ValueType::number) {
      return interpreter.allocateBoolean(number > r.number, line);
   }

   if (type == ValueType::string && r.type == ValueType::string) {
      const std::string &first  = interpreter.arena.strings[string];
      const std::string &second = interpreter.arena.strings[r.string];
      return interpreter.allocateBoolean(first > second, line);
   }

   raiseError(line, "Cannot compare %s value and %s value.",
      getValueTypeAsString(type), getValueTypeAsString(r.type));
}

ValueId Value::greaterEqual(Interpreter &interpreter, const Value &r) const {
   return r.smallerEqual(interpreter, *this);
}

ValueId Value::smaller(Interpreter &interpreter, const Value &r) const {
   return r.greater(interpreter, *this);
}

ValueId Value::smallerEqual(Interpreter &interpreter, const Value &r) const {
   ValueId isGreater = greater(interpreter, r);
   bool &result = interpreter.valuePool[isGreater].boolean;
   result = !result;
   return isGreater;
}

std::string Value::asString(Interpreter &interpreter) const {
   if (type == ValueType::null)    return "";
   if (type == ValueType::boolean) return (boolean ? "true" : "false");
   if (type == ValueType::string)  return interpreter.arena.strings[string];

   if (type == ValueType::number) {
      std::string string = std::to_string(number);
      return string.substr(0, string.size() - (number == floorl(number) ? 7 : 4));
   }
   raiseError(line, "Cannot convert %s value to String value.", getValueTypeAsString(type));
}

long double Value::asNumber(Interpreter &interpreter) const {
   if (type == ValueType::null)    return 0;
   if (type == ValueType::number)  return number;
   raiseError(line, "Cannot convert %s value to Number value.", getValueTypeAsString(type));
}

bool Value::asBoolean(Interpreter &interpreter) const {
   if (type == ValueType::null)    return false;
   if (type == ValueType::number)  return number != 0.0;
   if (type == ValueType::boolean) return boolean;
   if (type == ValueType::string)  return !interpreter.arena.strings[string].empty();
   raiseError(line, "Cannot convert %s value to Boolean value.", getValueTypeAsString(type));
}
