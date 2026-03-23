#include "error.hpp"
#include "interpreter.hpp"
#include "values.hpp"
#include <algorithm>
#include <cmath>

std::string Value::asPrintable(Interpreter &interpreter) {
   switch (type) {
   case ValueType::null:
      return "null";
   case ValueType::number:
   case ValueType::boolean:
   case ValueType::string:
      return asString(interpreter);
   case ValueType::function:
   case ValueType::ntFunction:
      return "[function]";
   case ValueType::array: {
      std::string result = "[ ";
      for (ValueId id: interpreter.arrayPool[array]) {
         Value &value = interpreter.valuePool[id];
         result += value.asPrintable(interpreter) + ", ";
      }
      return result + "]";
   } default:
      raiseError(line, "For some reason this value is not printable.");
   }
}

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

bool Value::equal(Interpreter &interpreter, const Value &r) const {
   if (type != r.type)              return false;
   if (type == ValueType::null)     return true;
   if (type == ValueType::number)   return number == r.number;
   if (type == ValueType::boolean)  return boolean == r.boolean;
   if (type == ValueType::function) return function.function == r.function.function;

   if (type == ValueType::array) {
      const std::vector<ValueId> &first  = interpreter.arrayPool[array];
      const std::vector<ValueId> &second = interpreter.arrayPool[r.array];
      if (first.size() != second.size()) return false;

      for (size_t i = 0; i < first.size(); ++i) {
         const Value &firstValue  = interpreter.valuePool[first[i]];
         const Value &secondValue = interpreter.valuePool[second[i]];

         if (!firstValue.equal(interpreter, secondValue)) {
            return false;
         }
      }
      return true;
   }

   const std::string &first  = interpreter.arena.strings[string];
   const std::string &second = interpreter.arena.strings[r.string];
   return first == second;
}

bool Value::greater(Interpreter &interpreter, const Value &r) const {
   if (type == ValueType::number && r.type == ValueType::number) {
      return number > r.number;
   }

   if (type == ValueType::array && r.type == ValueType::array) {
      const std::vector<ValueId> &first  = interpreter.arrayPool[array];
      const std::vector<ValueId> &second = interpreter.arrayPool[r.array];
      size_t size = std::min(first.size(), second.size());

      for (size_t i = 0; i < size; ++i) {
         const Value &firstValue  = interpreter.valuePool[first[i]];
         const Value &secondValue = interpreter.valuePool[second[i]];
         if (!firstValue.equal(interpreter, secondValue)) {
            return firstValue.greater(interpreter, secondValue);
         }
      }
      return first.size() > second.size();
   }

   if (type == ValueType::string && r.type == ValueType::string) {
      const std::string &first  = interpreter.arena.strings[string];
      const std::string &second = interpreter.arena.strings[r.string];
      std::string a = first;
      std::string b = second;

      std::transform(a.begin(), a.end(), a.begin(), ::tolower);
      std::transform(b.begin(), b.end(), b.begin(), ::tolower);

      return a > b;
   }

   raiseError(line, "Cannot compare %s value and %s value.",
      getValueTypeAsString(type), getValueTypeAsString(r.type));
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
   if (type == ValueType::array)   return true;
   raiseError(line, "Cannot convert %s value to Boolean value.", getValueTypeAsString(type));
}
