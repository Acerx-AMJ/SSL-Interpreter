#include "interpreter.hpp"
#include "error.hpp"
#include "nativeFunctions.hpp"

Interpreter::Interpreter(ASTArena &arena)
   : arena(arena) {
   arrayPool.reserve(128);
   valuePool.reserve(256);
   valuePool.push_back({}); // null
}

ValueId Interpreter::evaluate(NodeList program, Environment &environment) {
   ValueId last;
   for (size_t index = program.start; index < program.start + program.size; ++index)
      last = evaluateStmt(environment, arena.children[index]);
   return last;
}

ValueId Interpreter::callFunction(Environment &environment, NodeId function, const std::vector<ValueId> &args, size_t line) {
   if (arena.get(function).type == StmtType::identifier) {
      std::string &identifier = arena.strings[arena.get(function).identifier.id];

      if (isNativeFunction(identifier)) {
         NtFunc func = getNativeFunction(identifier, line);
         return func(args, *this, line);
      }
   }

   ValueId left = evaluateExpr(environment, function);
   Value &l = valuePool[left];

   if (l.type != ValueType::function) {
      raiseError(line, "Cannot call %s value.", getValueTypeAsString(l.type));
   }
   
   Node &stmt = arena.get(l.function.function);
   Environment *parent;
   NodeList *body, *params;

   if (stmt.type == StmtType::lambda) {
      parent = &environment;
      body = &stmt.lambda.body;
      params = &stmt.lambda.parameters;
   }
   else if (stmt.type == StmtType::fnDecl) {
      parent = l.function.env;
      body = &stmt.fnDecl.body;
      params = &stmt.fnDecl.parameters;
   }

   if (params->size != args.size()) {
      raiseError(line, "Expected argument count (%lu) to match parameter count (%lu).",
         args.size(), params->size);
   }

   Environment newEnvironment (parent);
   for (size_t i = 0; i < args.size(); ++i) {
      Node &param = arena.get(arena.children[i + params->start]);
      ValueId arg = args[i];

      newEnvironment.declare(*this, valuePool[arg].constant, arena.strings[param.identifier.id], arg, line);
   }
   return evaluate(*body, newEnvironment);
}

void Interpreter::callMain(Environment &global, int argc, char *argv[]) {
   if (!global.exists("main")) return;
   ValueId main = global.get("main", 0);
   Value &m = valuePool[main];

   if (m.type != ValueType::function) {
      raiseError(m.line, "Expected 'main' to be a Function but it is %s instead.",
         getValueTypeAsString(m.type));
   }

   Node &n = arena.get(m.function.function);
   Environment newEnvironment (&global);
   size_t argCount = n.fnDecl.parameters.size;

   if (n.type != StmtType::fnDecl) {
      raiseError(n.line, "Expected 'main' to be a Function but it is %s instead.",
         getStatementTypeAsString(n.type));
   }

   if (argCount > 1) {
      raiseError(n.line, "Expected 'main' to have 0 or 1 parameters but got %d instead.", argCount);
   }

   if (argCount == 1) {
      std::vector<ValueId> args;
      args.reserve(argc - 2);

      for (int i = 2; i < argc; ++i) {
         NodeId string = arena.allocateString(argv[i], n.line);
         ValueId stringValue = allocateString(arena.get(string).string.id, n.line);
         valuePool[stringValue].constant = true;
         args.push_back(stringValue);
      }

      std::string &identifier = arena.strings[arena.get(arena.children[n.fnDecl.parameters.start]).identifier.id];
      ValueId array = allocateArray(args, n.line);
      newEnvironment.declare(*this, true, identifier, array, n.line);
   }
   evaluate(n.fnDecl.body, newEnvironment);
}

// statement evaluation

ValueId Interpreter::evaluateStmt(Environment &environment, NodeId node) {
   Node &n = arena.get(node);

   switch (n.type) {
   case StmtType::varDecl: return evaluateVarDecl(environment, node);
   case StmtType::fnDecl:  return evaluateFnDecl(environment, node);
   default:                return evaluateExpr(environment, node);
   }
}

ValueId Interpreter::evaluateVarDecl(Environment &environment, NodeId node) {
   Node &n = arena.get(node);
   ValueId value = evaluateExpr(environment, n.varDecl.value);

   bool isConstant = n.varDecl.isConstant;
   bool isValueConstant = valuePool[value].constant;

   if (!isConstant && isValueConstant) {
      raiseError(n.line, "Attempted to declare a constant value as a mutable variable.");
   }
   
   environment.declare(*this, isConstant, arena.strings[n.varDecl.identifier], value, n.line);
   return null;
}

ValueId Interpreter::evaluateFnDecl(Environment &environment, NodeId node) {
   Node &n = arena.get(node);
   ValueId value = allocateFunction(node, &environment, n.line);
   environment.declare(*this, true, arena.strings[n.fnDecl.identifier], value, n.line);
   return null;
}

// expression evaluation

ValueId Interpreter::evaluateExpr(Environment &environment, NodeId node) {
   Node &n = arena.get(node);

   switch (n.type) {
   case StmtType::binary:         return evaluateBinaryExpr(environment, node);
   case StmtType::unary:          return evaluateUnaryExpr(environment, node);
   case StmtType::property:       return evaluatePropertyAccess(environment, node);
   case StmtType::arraySubscript: return evaluateArraySubscript(environment, node);
   case StmtType::assignment:     return evaluateAssignment(environment, node);
   case StmtType::call:           return evaluateCallExpr(environment, node);
   default:                       return evaluatePrimaryExpr(environment, node);
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
      if (!l.asBoolean(*this)) return allocateBoolean(false, n.line);
      ValueId right = evaluateExpr(environment, n.binary.right);
      bool result = valuePool[right].asBoolean(*this);
      return allocateBoolean(result, n.line);
   }
   else if (n.binary.op == TokenType::kor) {
      if (l.asBoolean(*this)) return allocateBoolean(true, n.line);
      ValueId right = evaluateExpr(environment, n.binary.right);
      bool result = valuePool[right].asBoolean(*this);
      return allocateBoolean(result, n.line);
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
   case TokenType::bigger:        return allocateBoolean(l.greater(*this, r), l.line);
   case TokenType::biggerEquals:  return allocateBoolean(!r.greater(*this, l), l.line);
   case TokenType::smaller:       return allocateBoolean(r.greater(*this, l), l.line);
   case TokenType::smallerEquals: return allocateBoolean(!l.greater(*this, r), l.line);
   case TokenType::equalsEquals:  return allocateBoolean(l.equal(*this, r), l.line);
   case TokenType::notEquals:     return allocateBoolean(!l.equal(*this, r), l.line);
   case TokenType::divisible: {
      ValueId result = l.remainder(*this, r);
      return allocateBoolean(!valuePool[result].asBoolean(*this), n.line);
   } default:
      raiseError(n.line, "Unsupported binary operator '%s'.", getTokenTypeAsString(n.binary.op));
   }
}

ValueId Interpreter::evaluateUnaryExpr(Environment &environment, NodeId node) {
   Node &n = arena.get(node);
   ValueId value = evaluateExpr(environment, n.unary.value);
   Value &v = valuePool[value];

   switch (n.unary.op) {
   case TokenType::plus:      return value;
   case TokenType::minus:     return v.negate(*this);
   case TokenType::knot:      return allocateBoolean(!v.asBoolean(*this), v.line);
   default: raiseError(n.line, "Unsupported unary operator '%s'.", getTokenTypeAsString(n.unary.op));
   }
}

ValueId Interpreter::evaluatePropertyAccess(Environment &environment, NodeId node) {

}

ValueId Interpreter::evaluateArraySubscript(Environment &environment, NodeId node) {
   Node &n = arena.get(node);
   ValueId left = evaluateExpr(environment, n.arraySubscript.left);
   Value &l = valuePool[left];

   ValueId right = evaluateExpr(environment, n.arraySubscript.expression);
   Value &r = valuePool[right];

   if (l.type != ValueType::array) {
      raiseError(n.line, "Expected Array value on the left side of the Array Subscript statement, "
                         "got %s instead.", getValueTypeAsString(l.type));
   }

   if (r.type != ValueType::number) {
      raiseError(n.line, "Expected Number value on the right side of the Array Subscript statement, "
                         "got %s instead.", getValueTypeAsString(r.type));
   }

   std::vector<ValueId> &array = arrayPool[l.array];
   size_t index = r.number;

   if (index >= array.size()) {
      raiseError(n.line, "Array Subscript index out of bounds. %lu (rounded from %s) >= %lu.",
         index, r.asString(*this).c_str(), array.size());
   }
   return array[index];
}

ValueId Interpreter::evaluateAssignment(Environment &environment, NodeId node) {
   Node &n = arena.get(node);
   ValueId right = evaluateExpr(environment, n.assignment.right);
   Value &r = valuePool[right];

   Node &leftnode = arena.get(n.assignment.left);

   // function calls aren't necessarily lvalues but they might be and I'm not in the mood to write the
   // whole logic for checking that 
   if (leftnode.type != StmtType::identifier     && leftnode.type != StmtType::property
    && leftnode.type != StmtType::arraySubscript && leftnode.type != StmtType::call) {
      raiseError(n.line, "Expected lvalue on the left of the assignment operator, got %s instead.",
         getStatementTypeAsString(leftnode.type));
   }

   ValueId left = evaluateExpr(environment, n.assignment.left);
   Value &l = valuePool[left];

   if (l.constant) {
      raiseError(n.line, "Cannot assign to a constant.");
   }

   switch (n.assignment.op) {
   case TokenType::equals:      l = r;                                break;
   case TokenType::plusEquals:  l = valuePool[l.add(*this, r)];       break;
   case TokenType::minusEquals: l = valuePool[l.subtract(*this, r)];  break;
   case TokenType::starEquals:  l = valuePool[l.multiply(*this, r)];  break;
   case TokenType::slashEquals: l = valuePool[l.divide(*this, r)];    break;
   case TokenType::modEquals:   l = valuePool[l.remainder(*this, r)]; break;
   case TokenType::powEquals:   l = valuePool[l.pow(*this, r)];       break;
   default:
      raiseError(n.line, "Unsupported assignment operator '%s'.", getTokenTypeAsString(n.assignment.op));
   }

   l.constant = r.constant;
   if (n.assignment.op != TokenType::equals) {
      valuePool.pop_back();
   }
   return left;
}

ValueId Interpreter::evaluateCallExpr(Environment &environment, NodeId node) {
   Node &n = arena.get(node);
   std::vector<ValueId> args;
   args.reserve(n.fnCall.args.size);

   for (size_t index = n.fnCall.args.start; index < n.fnCall.args.start + n.fnCall.args.size; ++index) {
      ValueId arg = evaluateExpr(environment, arena.children[index]);
      args.push_back(arg);
   }
   return callFunction(environment, n.fnCall.left, args, n.line);
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
      return allocateFunction(node, nullptr, n.line);
   case StmtType::program: {
      Environment child (&environment);
      return evaluate(n.program.nodes, child);
   } case StmtType::array: {
      NodeList &l = n.array.values;
      std::vector<ValueId> array;
      array.reserve(l.size);

      for (size_t i = l.start; i < l.start + l.size; ++i) {
         array.push_back(evaluateExpr(environment, arena.children[i]));
      }
      return allocateArray(array, n.line);
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

ValueId Interpreter::allocateFunction(NodeId function, Environment *environment, size_t line) {
   Value value {ValueType::function, line};
   value.function = {function, environment};
   return allocate(value);
}

ValueId Interpreter::allocateArray(const std::vector<ValueId> &array, size_t line) {
   Value value {ValueType::array, line};
   value.array = arrayPool.size();
   arrayPool.push_back(array);
   return allocate(value);
}
