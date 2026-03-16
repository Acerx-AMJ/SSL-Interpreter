#include "interpreter.hpp"
#include "error.hpp"

Interpreter::Interpreter(ASTArena &arena)
   : arena(arena) {
   valuePool.reserve(256);
   valuePool.push_back({}); // null
}

ValueId Interpreter::evaluate(NodeList program, Environment &environment) {
   ValueId last;
   for (size_t index = program.start; index < program.start + program.size; ++index) {
      last = evaluateStmt(environment, arena.children[index]);
      switch (valuePool[last].type) {
      case ValueType::number:
         printf("%Lf\n", valuePool[last].number);
         break;
      case ValueType::string:
         printf("%s\n", arena.strings[valuePool[last].string].c_str());
         break;
      case ValueType::boolean:
         printf("%s\n", valuePool[last].asString(*this).c_str());
         break;
      case ValueType::null:
         printf("null\n");
         break;
      case ValueType::function:
         printf("function/lambda\n");
         break;
      }
   }
   return last;
}

// statement evaluation

ValueId Interpreter::evaluateStmt(Environment &environment, NodeId node) {
   return evaluateExpr(environment, node);
}

// expression evaluation

ValueId Interpreter::evaluateExpr(Environment &environment, NodeId node) {
   Node &n = arena.get(node);

   switch (n.type) {
   case StmtType::binary:
      return evaluateBinaryExpr(environment, node);
   case StmtType::unary:
      return evaluateUnaryExpr(environment, node);
   case StmtType::property:
      return evaluatePropertyAccess(environment, node);
   case StmtType::arraySubscript:
      return evaluateArraySubscript(environment, node);
   case StmtType::assignment:
      return evaluateAssignment(environment, node);
   case StmtType::call:
      return evaluateCallExpr(environment, node);
   default:
      return evaluatePrimaryExpr(environment, node);
   }
}

ValueId Interpreter::evaluateBinaryExpr(Environment &environment, NodeId node) {
   Node &n = arena.get(node);
   ValueId left = evaluateExpr(environment, n.binary.left);
   Value &l = valuePool[left];

   if (n.binary.op == TokenType::nullOr) {
      return l.type == ValueType::null ? evaluateExpr(environment, n.binary.right) : left;
   }
   else if (n.binary.op == TokenType::kand) {
      if (!l.asBoolean(*this)) return false;
      ValueId right = evaluateExpr(environment, n.binary.right);
      return valuePool[right].asBoolean(*this);
   }
   else if (n.binary.op == TokenType::kor) {
      if (l.asBoolean(*this)) return true;
      ValueId right = evaluateExpr(environment, n.binary.right);
      return valuePool[right].asBoolean(*this);
   }

   ValueId right = evaluateExpr(environment, n.binary.right);
   Value &r = valuePool[right];

   switch (n.binary.op) {
   case TokenType::plus:          return l.add(*this, r);
   case TokenType::minus:         return l.subtract(*this, r);
   case TokenType::star:          return l.multiply(*this, r);
   case TokenType::slash:         return l.divide(*this, r);
   case TokenType::mod:           return l.remainder(*this, r);
   case TokenType::pow:           return l.pow(*this, r);
   case TokenType::bigger:        return l.greater(*this, r);
   case TokenType::biggerEquals:  return l.greaterEqual(*this, r);
   case TokenType::smaller:       return l.smaller(*this, r);
   case TokenType::smallerEquals: return l.smallerEqual(*this, r);
   case TokenType::equalsEquals:  return l.equal(*this, r);
   case TokenType::notEquals:     return l.notEqual(*this, r);
   case TokenType::divisible: {
      ValueId result = l.remainder(*this, r);
      return allocateBoolean(!valuePool[result].asBoolean(*this), l.line);
   }
   default: raiseError(l.line, "Unsupported binary operator '%s'.", getTokenTypeAsString(n.binary.op));
   }
}

ValueId Interpreter::evaluateUnaryExpr(Environment &environment, NodeId node) {

}

ValueId Interpreter::evaluatePropertyAccess(Environment &environment, NodeId node) {

}

ValueId Interpreter::evaluateArraySubscript(Environment &environment, NodeId node) {

}

ValueId Interpreter::evaluateAssignment(Environment &environment, NodeId node) {

}

ValueId Interpreter::evaluateCallExpr(Environment &environment, NodeId node) {

}

ValueId Interpreter::evaluatePrimaryExpr(Environment &environment, NodeId node) {
   Node &n = arena.get(node);

   switch (n.type) {
   case StmtType::identifier:
      return environment.get(arena.strings[n.identifier.id], n.line);
   case StmtType::number:
      return allocateNumber(n.number.number, n.line);
   case StmtType::string:
      return allocateString(n.string.id, n.line);
   case StmtType::null:
      return null;
   case StmtType::lambda:
      return allocateFunction(node, n.line);
   case StmtType::program: {
      Environment child (&environment);
      return evaluate(n.program.nodes, child);
   } default:
      raiseError(n.line, "Unexpected expression while evaluating: '%s'.",
         getStatementTypeAsString(n.type));
   }
}

// allocate

ValueId Interpreter::allocate(Value value) {
   size_t id = valuePool.size();
   valuePool.push_back(value);
   return id;
}

ValueId Interpreter::allocateNumber(long double number, size_t line) {
   Value value {ValueType::number, line};
   value.number = number;
   return allocate(value);
}

ValueId Interpreter::allocateBoolean(bool boolean, size_t line) {
   Value value {ValueType::boolean, line};
   value.boolean = boolean;
   return allocate(value);
}

ValueId Interpreter::allocateString(StringId string, size_t line) {
   Value value {ValueType::string, line};
   value.string = string;
   return allocate(value);
}

ValueId Interpreter::allocateString(const std::string &string, size_t line) {
   Value value {ValueType::string, line};
   value.string = arena.pushString(string);
   return allocate(value);
}

ValueId Interpreter::allocateFunction(NodeId function, size_t line) {
   Value value {ValueType::function, line};
   value.function = function;
   return allocate(value);
}
