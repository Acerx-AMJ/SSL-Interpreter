#ifndef SSL_INTERPRETER_HPP
#define SSL_INTERPRETER_HPP

#include "environment.hpp"

struct Interpreter {
   Interpreter(ASTArena &arena);

   ValueId evaluate(NodeList program, Environment &environment);
   ValueId callFunction(Environment &environment, NodeId function, const std::vector<ValueId> &args, size_t line);
   void callMain(Environment &environment, int argc, char *argv[]);

   // statement evaluation

   ValueId evaluateStmt(Environment &environment, NodeId node);
   ValueId evaluateVarDecl(Environment &environment, NodeId node);
   ValueId evaluateFnDecl(Environment &environment, NodeId node);
   ValueId evaluateIfStmt(Environment &environment, NodeId node);
   ValueId evaluateMatchStmt(Environment &environment, NodeId node);
   ValueId evaluateForLoop(Environment &environment, NodeId node);
   ValueId evaluateLoop(Environment &environment, NodeId node);
   ValueId evaluateWhileLoop(Environment &environment, NodeId node);
   ValueId evaluateDoWhileLoop(Environment &environment, NodeId node);
   ValueId evaluateBreak(Environment &environment, NodeId node);
   ValueId evaluateContinue(Environment &environment, NodeId node);
   ValueId evaluateReturnStmt(Environment &environment, NodeId node);
   ValueId evaluateUnlessStmt(Environment &environment, NodeId node);

   // expression evaluation

   ValueId evaluateExpr(Environment &environment, NodeId node);
   ValueId evaluateBinaryExpr(Environment &environment, NodeId node);
   ValueId evaluateUnaryExpr(Environment &environment, NodeId node);
   ValueId evaluatePropertyAccess(Environment &environment, NodeId node);
   ValueId evaluateArraySubscript(Environment &environment, NodeId node);
   ValueId evaluateAssignment(Environment &environment, NodeId node);
   ValueId evaluateCallExpr(Environment &environment, NodeId node);
   ValueId evaluatePrimaryExpr(Environment &environment, NodeId node);

   // allocate

   ValueId allocate(Value value);
   ValueId allocateNumber(long double number, size_t line);
   ValueId allocateBoolean(bool boolean, size_t line);
   ValueId allocateString(StringId string, size_t line);
   ValueId allocateString(const std::string &string, size_t line);
   ValueId allocateFunction(NodeId function, Environment *environment, size_t line);
   ValueId allocateArray(const std::vector<ValueId> &array, size_t line);
   ValueId copy(ValueId id, bool arrayList = false);

   // Members

   ASTArena &arena;
   std::vector<Value> valuePool;
   std::vector<std::vector<ValueId>> arrayPool;

   bool returning = false;
};

#endif
