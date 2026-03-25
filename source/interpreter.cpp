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
   for (size_t index = program.start; index < program.start + program.size; ++index) {
      last = evaluateStmt(environment, arena.children[index]);
      if (returning) return returnValue;
   }
   return last;
}

ValueId Interpreter::callFunction(Environment &environment, NodeId function, const std::vector<ValueId> &args, size_t line) {
   ValueId left = evaluateExpr(environment, function);
   Value &l = valuePool[left];

   if (l.type == ValueType::ntFunction) {
      return l.nativeFn(args, *this, line);
   }

   if (l.type != ValueType::function) {
      raiseError(line, "Cannot call %s value.", getValueTypeAsString(l.type));
   }
   
   Node &stmt = arena.get(l.function.function);
   Environment *parent;
   NodeList *body, *params;
   bool returnsRef;

   if (stmt.type == StmtType::lambda) {
      parent = &environment;
      body = &stmt.lambda.body;
      params = &stmt.lambda.parameters;
      returnsRef = stmt.lambda.returnsRef;
   }
   else if (stmt.type == StmtType::fnDecl) {
      parent = l.function.env;
      body = &stmt.fnDecl.body;
      params = &stmt.fnDecl.parameters;
      returnsRef = stmt.fnDecl.returnsRef;
   }

   if (params->size != args.size()) {
      raiseError(line, "Expected argument count (%lu) to match parameter count (%lu).",
         args.size(), params->size);
   }

   Environment newEnvironment (parent);
   for (size_t i = 0; i < args.size(); ++i) {
      Node &param = arena.get(arena.children[i + params->start]);
      ValueId arg = param.ref ? args[i] : copy(args[i]);
      valuePool[arg].lvalue = param.ref;

      newEnvironment.declare(*this, valuePool[arg].constant, arena.strings[param.identifier.id], arg, line);
   }

   bool previousReturning = returning;
   returning = false;
   ValueId result = evaluate(*body, newEnvironment);
   returning = previousReturning;

   valuePool[result].lvalue = returnsRef;
   return returnsRef ? result : copy(result);
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
         ValueId stringValue = allocateString(argv[i], n.line);
         valuePool[stringValue].constant = true;
         args.push_back(stringValue);
      }

      std::string &identifier = arena.strings[arena.get(arena.children[n.fnDecl.parameters.start]).identifier.id];
      ValueId array = allocateArray(args, n.line);
      newEnvironment.declare(*this, true, identifier, array, n.line);
   }

   evaluate(n.fnDecl.body, newEnvironment);
   returning = false;
}

// statement evaluation

ValueId Interpreter::evaluateStmt(Environment &environment, NodeId node) {
   Node &n = arena.get(node);

   switch (n.type) {
   case StmtType::varDecl:      return evaluateVarDecl(environment, node);
   case StmtType::fnDecl:       return evaluateFnDecl(environment, node);
   case StmtType::ifStmt:       return evaluateIfStmt(environment, node);
   case StmtType::matchStmt:    return evaluateMatchStmt(environment, node);
   case StmtType::forLoop:      return evaluateForLoop(environment, node);
   case StmtType::loop:         return evaluateLoop(environment, node);
   case StmtType::whileLoop:    return evaluateWhileLoop(environment, node);
   case StmtType::doWhileLoop:  return evaluateDoWhileLoop(environment, node);
   case StmtType::breakStmt:    return evaluateBreak(environment, node);
   case StmtType::continueStmt: return evaluateContinue(environment, node);
   case StmtType::returnStmt:   return evaluateReturnStmt(environment, node);
   case StmtType::unlessStmt:   return evaluateUnlessStmt(environment, node);
   default:                     return evaluateExpr(environment, node);
   }
}

ValueId Interpreter::evaluateVarDecl(Environment &environment, NodeId node) {
   Node &n = arena.get(node);
   ValueId value = evaluateExpr(environment, n.varDecl.value);
   bool isConstant = n.varDecl.isConstant;
   bool isValueConstant = valuePool[value].constant && valuePool[value].ref;

   if (!isConstant && isValueConstant) {
      raiseError(n.line, "Attempted to declare a constant value as a mutable reference. Change := to "
                         ":: to make it constant.");
   }

   ValueId arg = valuePool[value].ref ? value : copy(value);
   environment.declare(*this, isConstant, arena.strings[n.varDecl.identifier], arg, n.line);
   return null;
}

ValueId Interpreter::evaluateFnDecl(Environment &environment, NodeId node) {
   Node &n = arena.get(node);
   ValueId value = allocateFunction(node, &environment, n.line);
   environment.declare(*this, true, arena.strings[n.fnDecl.identifier], value, n.line);
   return null;
}

ValueId Interpreter::evaluateIfStmt(Environment &environment, NodeId node) {
   Node &n = arena.get(node);
   Node &ifs = arena.get(n.ifStmt.ifClause);

   ValueId ifResult = evaluateExpr(environment, ifs.ifClause.expression);
   Value &result = valuePool[ifResult];

   if (result.asBoolean(*this)) {
      Environment newEnvironment (&environment);
      return evaluate(ifs.ifClause.statement, newEnvironment);
   }

   NodeList elifs = n.ifStmt.elifClauses;
   for (size_t i = elifs.start; i < elifs.start + elifs.size; ++i) {
      Node &elif = arena.get(arena.children[i]);
      ValueId elifResult = evaluateExpr(environment, elif.ifClause.expression);
      Value &result = valuePool[elifResult];

      if (result.asBoolean(*this)) {
         Environment newEnvironment (&environment);
         return evaluate(elif.ifClause.statement, newEnvironment);
      }
   }

   if (n.ifStmt.elseClause == null) {
      return null;
   }

   Node &elses = arena.get(n.ifStmt.elseClause);
   Environment newEnvironment (&environment);
   return evaluate(elses.ifClause.statement, newEnvironment);
}

ValueId Interpreter::evaluateMatchStmt(Environment &environment, NodeId node) {
   Node &n = arena.get(node);
   NodeList cases = n.matchStmt.cases;

   ValueId expression = evaluateExpr(environment, n.matchStmt.expression);
   Value &expr = valuePool[expression];

   for (size_t i = cases.start; i < cases.start + cases.size; ++i) {
      Node &mcase = arena.get(arena.children[i]);
      Node &range = arena.get(mcase.ifClause.expression);
      
      if (range.type == StmtType::range) {
         if (expr.type != ValueType::number) {
            raiseError(mcase.line, "Range can only be used to check against number values.");
         }

         ValueId left = evaluateExpr(environment, range.range.left);
         ValueId right = evaluateExpr(environment, range.range.right);

         Value &l = valuePool[left];
         Value &r = valuePool[right];

         if (l.type != ValueType::number || r.type != ValueType::number) {
            raiseError(range.line, "Expected both left and right side of the range statement to be "
                                   "numbers.");
         }

         if (!range.range.inclusive) {
            r.number -= 1;
         }

         if (l.number > r.number) {
            std::swap(l, r);
         }

         if (expr.number >= l.number && expr.number <= r.number) {
            Environment newEnvironment (&environment);
            return evaluate(mcase.ifClause.statement, newEnvironment);
         }
         continue;
      }

      ValueId caseResult = evaluateExpr(environment, mcase.ifClause.expression);
      Value &result = valuePool[caseResult];

      if (expression == null) {
         if (result.asBoolean(*this)) {
            Environment newEnvironment (&environment);
            return evaluate(mcase.ifClause.statement, newEnvironment);
         }
         continue;
      }

      if ((result.type == ValueType::boolean && result.boolean)
       || (result.type != ValueType::boolean && result.equal(*this, expr))) {
         Environment newEnvironment (&environment);
         return evaluate(mcase.ifClause.statement, newEnvironment);
      }
   }
   Environment newEnvironment (&environment);
   return evaluate(arena.get(n.matchStmt.elseClause).ifClause.statement, newEnvironment);
}

ValueId Interpreter::evaluateForLoop(Environment &environment, NodeId node) {
   Node &n = arena.get(node);
   Node &inExpr = arena.get(n.forLoop.inExpression);
   std::string &identifier = arena.strings[n.forLoop.identifier];

   if (inExpr.type == StmtType::range) {
      if (!arena.strings[n.forLoop.indexIdentifier].empty()) {
         raiseError(n.line, "Expected only one identifier in for range loop.");
      }

      ValueId left = evaluateExpr(environment, inExpr.range.left);
      ValueId right = evaluateExpr(environment, inExpr.range.right);

      Value &l = valuePool[left];
      Value &r = valuePool[right];

      if (l.type != ValueType::number || r.type != ValueType::number) {
         raiseError(inExpr.line, "Expected both left and right side of the range statement to be "
                                 "numbers.");
      }

      bool reversed = n.forLoop.reversed;
      if (!inExpr.range.inclusive) {
         r.number += l.number > r.number ? 1 : -1;
      }
      
      if (l.number > r.number) {
         std::swap(l, r);
         reversed = !reversed;
      }

      for (long double i = reversed ? r.number : l.number; reversed ? (i >= l.number) : (i <= r.number); reversed ? --i : ++i) {
         Environment newEnvironment (&environment);
         newEnvironment.declare(*this, true, identifier, allocateNumber(i, n.line), n.line);
         evaluate(n.forLoop.body, newEnvironment);
         if (returning) return null;
      }
      return null;
   }

   // handle iterating strings and arrays
   std::string indexIdentifier = arena.strings[n.forLoop.indexIdentifier];
   ValueId expression = evaluateExpr(environment, n.forLoop.inExpression);
   Value &expr = valuePool[expression];

   if (expr.type == ValueType::array) {
      bool reversed = n.forLoop.reversed;
      std::vector<ValueId> &array = arrayPool[expr.array];

      for (long i = reversed ? array.size() - 1 : 0; reversed ? (i >= 0) : (i < array.size()); reversed ? --i : ++i) {
         Environment newEnvironment (&environment);
         newEnvironment.declare(*this, valuePool[array[i]].constant, identifier, array[i], n.line);

         if (!indexIdentifier.empty()) {
            newEnvironment.declare(*this, true, indexIdentifier, allocateNumber(i, n.line), n.line);
         }
         evaluate(n.forLoop.body, newEnvironment);
         if (returning) return null;
      }
      return null;
   }
   else if (expr.type == ValueType::string) {
      bool reversed = n.forLoop.reversed;
      std::string &string = arena.strings[expr.string];

      for (long i = reversed ? string.size() - 1 : 0; reversed ? (i >= 0) : (i < string.size()); reversed ? --i : ++i) {
         Environment newEnvironment (&environment);
         newEnvironment.declare(*this, false, identifier, allocateCharacter(expr.string, i, n.line), n.line);

         if (!indexIdentifier.empty()) {
            newEnvironment.declare(*this, true, indexIdentifier, allocateNumber(i, n.line), n.line);
         }
         evaluate(n.forLoop.body, newEnvironment);
         if (returning) return null;
      }
      return null;
   }
   else {
      raiseError(expr.line, "Cannot iterate over %s value.", getValueTypeAsString(expr.type));
   }
}

ValueId Interpreter::evaluateLoop(Environment &environment, NodeId node) {
   Node &n = arena.get(node);
   while (true) {
      Environment newEnvironment (&environment);
      evaluate(n.loop.body, newEnvironment);
      if (returning) return null;
   }
   return null;
}

ValueId Interpreter::evaluateWhileLoop(Environment &environment, NodeId node) {
   Node &n = arena.get(node);
   while (true) {
      ValueId expression = evaluateExpr(environment, n.whileLoop.expression);
      Value &expr = valuePool[expression];
      if (!expr.asBoolean(*this)) return null;

      Environment newEnvironment (&environment);
      evaluate(n.whileLoop.statement, newEnvironment);
      if (returning) return null;
   }
   return null;
}

ValueId Interpreter::evaluateDoWhileLoop(Environment &environment, NodeId node) {
   Node &n = arena.get(node);
   while (true) {
      Environment newEnvironment (&environment);
      evaluate(n.whileLoop.statement, newEnvironment);

      ValueId expression = evaluateExpr(environment, n.whileLoop.expression);
      Value &expr = valuePool[expression];
      if (returning || !expr.asBoolean(*this)) return null;
   }
   return null;
}

ValueId Interpreter::evaluateBreak(Environment &environment, NodeId node) {

}

ValueId Interpreter::evaluateContinue(Environment &environment, NodeId node) {
   
}

ValueId Interpreter::evaluateReturnStmt(Environment &environment, NodeId node) {
   Node &returnStmt = arena.get(node);
   returning = true;
   returnValue = evaluateExpr(environment, returnStmt.returnStmt.value);
   return null;
}

ValueId Interpreter::evaluateUnlessStmt(Environment &environment, NodeId node) {
   Node &unlessStmt = arena.get(node);
   ValueId expression = evaluateExpr(environment, unlessStmt.unlessStmt.expression);
   Value &expr = valuePool[expression];

   if (expr.asBoolean(*this)) {
      return null;
   }
   return evaluateStmt(environment, unlessStmt.unlessStmt.statement);
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
   case TokenType::kin:           return evaluateInExpr(environment, l, r);
   case TokenType::kas:           return evaluateTypeCast(environment, l, r);
   case TokenType::kis:           return evaluateTypeChecking(environment, l, r);
   case TokenType::divisible: {
      ValueId result = l.remainder(*this, r);
      return allocateBoolean(!valuePool[result].asBoolean(*this), n.line);
   } default:
      raiseError(n.line, "Unsupported binary operator '%s'.", getTokenTypeAsString(n.binary.op));
   }
}

ValueId Interpreter::evaluateInExpr(Environment &environment, Value &left, Value &right) {

}

ValueId Interpreter::evaluateTypeCast(Environment &environment, Value &left, Value &right) {
   if (right.type != ValueType::type) {
      raiseError(right.line, "Expected Type value on the left of as expression but got '%s' instead.",
         getValueTypeAsString(right.type));
   }

   switch (right.vtype) {
   case ValueType::number:
      return allocateNumber(left.asNumber(*this), left.line);
   case ValueType::boolean:
      return allocateBoolean(left.asBoolean(*this), left.line);
   case ValueType::character:
      return allocateCharacter(left.asChar(*this), left.line);
   case ValueType::string:
      return allocateString(left.asString(*this), left.line);
   default:
      raiseError(right.line, "Cannot cast a value to %s value.", getValueTypeAsString(right.vtype));
   }
}

ValueId Interpreter::evaluateTypeChecking(Environment &environment, Value &left, Value &right) {
   if (right.type != ValueType::type) {
      raiseError(right.line, "Expected Type value on the left of is expression but got '%s' instead.",
         getValueTypeAsString(right.type));
   }

   // native functions and functions are the same type!
   ValueType type = left.type == ValueType::ntFunction ? ValueType::function : left.type;
   return type == right.vtype;
}

ValueId Interpreter::evaluateUnaryExpr(Environment &environment, NodeId node) {
   Node &n = arena.get(node);
   ValueId value = evaluateExpr(environment, n.unary.value);
   Value &v = valuePool[value];

   switch (n.unary.op) {
   case TokenType::plus:      return value;
   case TokenType::minus:     return v.negate(*this);
   case TokenType::knot:      return allocateBoolean(!v.asBoolean(*this), v.line);
   case TokenType::ref: {
      if (!v.lvalue) {
         raiseError(n.line, "Expected lvalue to the right of the & operator.");
      }
      v.ref = true;
      return value;
   } default:
      raiseError(n.line, "Unsupported unary operator '%s'.", getTokenTypeAsString(n.unary.op));
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

   if (r.type != ValueType::number) {
      raiseError(n.line, "Expected Number value on the right side of the Array Subscript statement, "
                         "got %s instead.", getValueTypeAsString(r.type));
   }

   if (l.type == ValueType::array) {
      std::vector<ValueId> &array = arrayPool[l.array];
      size_t index = r.number;

      if (index >= array.size()) {
         raiseError(n.line, "Array Subscript index out of bounds. %lu (rounded from %s) >= %lu.",
            index, r.asString(*this).c_str(), array.size());
      }
      return array[index];
   }
   else if (l.type == ValueType::string) {
      std::string string = arena.strings[l.string];
      size_t index = r.number;

      if (index >= string.size()) {
         raiseError(n.line, "Array Subscript index out of bounds. %lu (rounded from %s) >= %lu.",
            index, r.asString(*this).c_str(), string.size());
      }

      ValueId character = allocateCharacter(l.string, index, l.line);
      valuePool[character].lvalue = true; 
      valuePool[character].constant = l.constant;
      return character;
   }
   else {
      raiseError(n.line, "Expected Array/String value on the left side of the Array Subscript "
                         "statement, got %s instead.", getValueTypeAsString(l.type));
   }
}

ValueId Interpreter::evaluateAssignment(Environment &environment, NodeId node) {
   Node &n = arena.get(node);
   ValueId right = evaluateExpr(environment, n.assignment.right);
   Value &r = valuePool[right];

   Node &leftnode = arena.get(n.assignment.left);

   ValueId left = evaluateExpr(environment, n.assignment.left);
   Value &l = valuePool[left];

   if (!l.lvalue) {
      raiseError(n.line, "Expected lvalue on the left of the assignment operator.");
   }

   if (l.constant) {
      raiseError(n.line, "Cannot assign to a constant.");
   }

   // special handling for characters since they can't be easily modified
   if (l.type == ValueType::character) {
      char &ch = arena.strings[l.character.stringId][l.character.index];
      
      switch (n.assignment.op) {
      case TokenType::equals:      ch = r.asChar(*this);                                break;
      case TokenType::plusEquals:  ch = valuePool[l.add(*this, r)].asChar(*this);       break;
      case TokenType::minusEquals: ch = valuePool[l.subtract(*this, r)].asChar(*this);  break;
      case TokenType::starEquals:  ch = valuePool[l.multiply(*this, r)].asChar(*this);  break;
      case TokenType::slashEquals: ch = valuePool[l.divide(*this, r)].asChar(*this);    break;
      case TokenType::modEquals:   ch = valuePool[l.remainder(*this, r)].asChar(*this); break;
      case TokenType::powEquals:   ch = valuePool[l.pow(*this, r)].asChar(*this);       break;
      default:
         raiseError(n.line, "Unsupported assignment operator '%s'.", getTokenTypeAsString(n.assignment.op));
      }
   }
   else {
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
   }

   l.constant = r.constant && r.type != ValueType::ntFunction;
   l.lvalue = true; // let's hope this doesn't turn into something the users will abuse
   
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
         ValueId value = evaluateExpr(environment, arena.children[i]);
         valuePool[value].lvalue = true;
         array.push_back(value);
      }
      return allocateArray(array, n.line);
   } default:
      raiseError(n.line, "Unexpected expression while evaluating: '%s'.",
         getStatementTypeAsString(n.type));
   }
}

// allocate

ValueId Interpreter::allocate(const Value &value) {
   size_t id = valuePool.size();
   valuePool.push_back(value);
   return id;
}

ValueId Interpreter::allocateNumber(long double number, size_t line) {
   Value value;
   value.type = ValueType::number;
   value.line = line;
   value.number = number;
   return allocate(value);
}

ValueId Interpreter::allocateBoolean(bool boolean, size_t line) {
   Value value;
   value.type = ValueType::boolean;
   value.line = line;
   value.boolean = boolean;
   return allocate(value);
}

ValueId Interpreter::allocateCharacter(StringId stringId, size_t index, size_t line) {
   Value value;
   value.type = ValueType::character;
   value.line = line;
   value.character = {stringId, index};
   return allocate(value);
}

ValueId Interpreter::allocateCharacter(char ch, size_t line) {
   Value value;
   value.type = ValueType::character;
   value.line = line;
   value.character = {0, arena.strings[0].size()};
   arena.strings[0].push_back(ch);
   return allocate(value);
}

ValueId Interpreter::allocateString(StringId string, size_t line) {
   Value value;
   value.type = ValueType::string;
   value.line = line;
   value.string = string;
   return allocate(value);
}

ValueId Interpreter::allocateString(const std::string &string, size_t line) {
   Value value;
   value.type = ValueType::string;
   value.line = line;
   value.string = arena.pushString(string);
   return allocate(value);
}

ValueId Interpreter::allocateFunction(NodeId function, Environment *environment, size_t line) {
   Value value;
   value.type = ValueType::function;
   value.line = line;
   value.function = {function, environment};
   return allocate(value);
}

ValueId Interpreter::allocateNtFunction(NtFunc function) {
   Value value;
   value.type = ValueType::ntFunction;
   value.line = 0;
   value.nativeFn = function;
   return allocate(value);
}

ValueId Interpreter::allocateArray(const std::vector<ValueId> &array, size_t line) {
   Value value;
   value.type = ValueType::array;
   value.line = line;
   value.array = arrayPool.size();
   arrayPool.push_back(array);
   return allocate(value);
}

ValueId Interpreter::allocateType(ValueType type, size_t line) {
   Value value;
   value.type = ValueType::type;
   value.line = line;
   value.vtype = type;
   return allocate(value);
}

ValueId Interpreter::copy(ValueId id, bool arrayList) {
   Value value = valuePool[id];
   value.lvalue = arrayList;
   value.constant = false;

   if (value.type != ValueType::array) return allocate(value);

   std::vector<ValueId> &array = arrayPool[value.array];
   std::vector<ValueId> newArray;
   newArray.reserve(array.size());

   for (ValueId id: array) {
      newArray.push_back(copy(id, true));
   }
   value.array = arrayPool.size();
   arrayPool.push_back(newArray);
   return allocate(value);
}
