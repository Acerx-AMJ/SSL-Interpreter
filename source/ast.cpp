#include "ast.hpp"

ASTArena::ASTArena() {
   nodes.reserve(256);
   children.reserve(256);
   strings.reserve(128);
   nodes.push_back({}); // 0 - invalid
}

StringId ASTArena::pushString(const std::string &string) {
   StringId index = strings.size();
   strings.push_back(string);
   return index;
}

std::string_view ASTArena::getString(StringId id) const {
   return strings[id];
}

NodeList ASTArena::pushVector(const std::vector<NodeId> &values) {
   NodeList nodeList {children.size(), values.size()};
   for (size_t i = 0; i < values.size(); ++i) {
      children.push_back(values[i]);
   }
   return nodeList;
}

Node &ASTArena::get(NodeId id) {
   return nodes[id];
}

const Node &ASTArena::get(NodeId id) const {
   return nodes[id];
}

void ASTArena::printList(NodeList list, int indentation) const {
   putchar('\n');
   for (size_t i = list.start; i < list.size + list.start; ++i) {
      print(children[i], indentation + 1);
   }

   if (indentation == 0) {
      putchar('\n');
   }
}

void ASTArena::print(NodeId id, int indentation) const {
   const Node &node = get(id);
   printf("\n%*s%s: ", indentation, "", getStatementTypeAsString(node.type));

   switch (node.type) {
     case StmtType::varDecl: {
      printf("%s:", strings[node.varDecl.identifier].c_str());
      print(node.varDecl.value, indentation + 1);
      break;
   } case StmtType::fnDecl: {
      printf("%s:", strings[node.fnDecl.identifier].c_str());
      printf("\n%*sParameters:", indentation + 1, "");
      printList(node.fnDecl.parameters, indentation + 1);
      print(node.fnDecl.body, indentation + 1);
      break;
   } case StmtType::lambda: {
      printf("\n%*sParameters:", indentation + 1, "");
      printList(node.lambda.parameters, indentation + 1);
      print(node.lambda.body, indentation + 1);
      break;
   } case StmtType::enumDecl: {
      printf("%s:", strings[node.enumDecl.identifier].c_str());
      printf("\n%*sEntries:", indentation + 1, "");
      printList(node.enumDecl.entries, indentation + 1);
      break;
   } case StmtType::structDecl: {
      printf("%s:", strings[node.structDecl.identifier].c_str());
      printf("\n%*sDeclarations:", indentation + 1, "");
      printList(node.structDecl.varDeclarations, indentation + 1);
      break;
   } case StmtType::ifStmt: {
      print(node.ifStmt.ifClause, indentation + 1);
      printList(node.ifStmt.elifClauses, indentation + 1);
      print(node.ifStmt.elseClause, indentation + 1);
      break;
   } case StmtType::ifClause: {
      printf("%s:", getKeywordTypeAsString(node.ifClause.keyword));
      print(node.ifClause.expression, indentation + 1);
      print(node.ifClause.statement, indentation + 1);
      break;
   } case StmtType::matchStmt: {
      printf("\n%*sCases:", indentation + 1, "");
      printList(node.matchStmt.cases, indentation + 1);
      print(node.matchStmt.elseClause, indentation + 1);
      break;
   } case StmtType::matchCase: {
      print(node.matchCase.expression, indentation + 1);
      print(node.matchCase.statement, indentation + 1);
      break;
   } case StmtType::forLoop: {
      printf("%s:", strings[node.forLoop.identifier].c_str());
      print(node.forLoop.inExpression, indentation + 1);
      print(node.forLoop.body, indentation + 1);
      break;
   } case StmtType::loop: {
      print(node.loop.body, indentation + 1);
      break;
   } case StmtType::whileLoop: {
      print(node.whileLoop.expression, indentation + 1);
      print(node.whileLoop.statement, indentation + 1);
      break;
   } case StmtType::doWhileLoop: {
      print(node.doWhileLoop.expression, indentation + 1);
      print(node.doWhileLoop.statement, indentation + 1);
      break;
   } case StmtType::breakStmt: {
      break;
   } case StmtType::continueStmt: {
      break;
   } case StmtType::returnStmt: {
      print(node.returnStmt.value, indentation + 1);
      break;
   } case StmtType::unlessStmt: {
      print(node.unlessStmt.statement, indentation + 1);
      print(node.unlessStmt.expression, indentation + 1);
      break;
   } case StmtType::doStmt: {
      print(node.doStmt.statement, indentation + 1);
      break;
   } case StmtType::importStmt: {
      printf("%s:", strings[node.importStmt.file].c_str());
      printf("\n%*sValues:", indentation + 1, "");
      printList(node.importStmt.values, indentation + 2);
      print(node.importStmt.as, indentation + 1);
      break;
   } case StmtType::assignment: {
      printf("\n%*sOperator: %s", indentation + 1, "", getTokenTypeAsString(node.assignment.op));
      print(node.assignment.left, indentation + 2);
      print(node.assignment.right, indentation + 2);
      break;
   } case StmtType::binary: {
      printf("\n%*sOperator: %s", indentation + 1, "", getTokenTypeAsString(node.binary.op));
      print(node.binary.left, indentation + 2);
      print(node.binary.right, indentation + 2);
      break;
   } case StmtType::unary: {
      printf("\n%*sOperator: %s", indentation + 1, "", getTokenTypeAsString(node.unary.op));
      print(node.unary.value, indentation + 2);
      break;
   } case StmtType::property: {
      print(node.property.left, indentation + 1);
      printList(node.property.right, indentation + 1);
      break;
   } case StmtType::call: {
      printf("%s:", strings[node.fnCall.identifier].c_str());
      printf("\n%*sArguments:", indentation + 1, "");
      printList(node.fnCall.args, indentation + 2);
      break;
   } case StmtType::enumEntry: {
      printf("%s:", strings[node.enumEntry.identifier].c_str());
      print(node.enumEntry.value, indentation + 1);
      printf("\n%*sArguments:", indentation + 1, "");
      printList(node.enumEntry.args, indentation + 2);
      printf("\n%*sArgument Values:", indentation + 1, "");
      printList(node.enumEntry.argValues, indentation + 2);
      break;
   } case StmtType::identifier: {
      printf("%s", strings[node.identifier.id].c_str());
      break;
   } case StmtType::number: {
      printf("%Lf", node.number.number);
      break;
   } case StmtType::string: {
      printf("%s", strings[node.string.id].c_str());
      break;
   } case StmtType::array: {
      printList(node.array.values, indentation + 1);
      break;
   } case StmtType::null: {
      break;
   } case StmtType::program: {
      printList(node.program.nodes, indentation + 1);
      break;
   }}

   if (indentation == 0) {
      putchar('\n');
   }
}

// builders

NodeId ASTArena::allocate(Node node) {
   NodeId index = nodes.size();
   nodes.push_back(node);
   return index;
}

NodeId ASTArena::allocateVarDecl(bool isPublic, const std::string &identifier, NodeId value, size_t line) {
   Node node {StmtType::varDecl, line};
   node.varDecl = {isPublic, pushString(identifier), value};
   return allocate(node);
}

NodeId ASTArena::allocateFnDecl(bool isPublic, const std::string &identifier, NodeId body, const std::vector<NodeId> &params, size_t line) {
   Node node {StmtType::fnDecl, line};
   node.fnDecl = {isPublic, pushString(identifier), body, pushVector(params)};
   return allocate(node);
}

NodeId ASTArena::allocateLambda(NodeId body, const std::vector<NodeId> &params, size_t line) {
   Node node {StmtType::lambda, line};
   node.lambda = {body, pushVector(params)};
   return allocate(node);
}

NodeId ASTArena::allocateEnumDecl(bool isPublic, const std::string &identifier, const std::vector<NodeId> &entries, size_t line) {
   Node node {StmtType::enumDecl, line};
   node.enumDecl = {isPublic, pushString(identifier), pushVector(entries)};
   return allocate(node);
}

NodeId ASTArena::allocateStructDecl(bool isPublic, const std::string &identifier, const std::vector<NodeId> &decls, size_t line) {
   Node node {StmtType::structDecl, line};
   node.structDecl = {isPublic, pushString(identifier), pushVector(decls)};
   return allocate(node);
}

NodeId ASTArena::allocateIfStmt(NodeId ifClause, const std::vector<NodeId> &elifClauses, NodeId elseClause, size_t line) {
   Node node {StmtType::ifStmt, line};
   node.ifStmt = {ifClause, pushVector(elifClauses), elseClause};
   return allocate(node);
}

NodeId ASTArena::allocateIfClause(KeywordType keyword, NodeId expression, NodeId statement, size_t line) {
   Node node {StmtType::ifClause, line};
   node.ifClause = {keyword, expression, statement};
   return allocate(node);
}

NodeId ASTArena::allocateMatchStmt(const std::vector<NodeId> &cases, NodeId elseClause, size_t line) {
   Node node {StmtType::matchStmt, line};
   node.matchStmt = {pushVector(cases), elseClause};
   return allocate(node);
}

NodeId ASTArena::allocateMatchCase(NodeId expression, NodeId statement, size_t line) {
   Node node {StmtType::matchCase, line};
   node.matchCase = {expression, statement};
   return allocate(node);
}

NodeId ASTArena::allocateForLoop(const std::string &identifier, NodeId inExpression, NodeId body, size_t line) {
   Node node {StmtType::forLoop, line};
   node.forLoop = {pushString(identifier), inExpression, body};
   return allocate(node);
}

NodeId ASTArena::allocateLoop(NodeId body, size_t line) {
   Node node {StmtType::loop, line};
   node.loop = {body};
   return allocate(node);
}

NodeId ASTArena::allocateWhileLoop(NodeId expression, NodeId statement, size_t line) {
   Node node {StmtType::whileLoop, line};
   node.whileLoop = {expression, statement};
   return allocate(node);
}

NodeId ASTArena::allocateDoWhileLoop(NodeId expression, NodeId statement, size_t line) {
   Node node {StmtType::doWhileLoop, line};
   node.doWhileLoop = {expression, statement};
   return allocate(node);
}

NodeId ASTArena::allocateBreakStmt(size_t line) {
   Node node {StmtType::breakStmt, line};
   node.breakStmt = {};
   return allocate(node);
}

NodeId ASTArena::allocateContinueStmt(size_t line) {
   Node node {StmtType::continueStmt, line};
   node.continueStmt = {};
   return allocate(node);
}

NodeId ASTArena::allocateReturnStmt(NodeId value, size_t line) {
   Node node {StmtType::returnStmt, line};
   node.returnStmt = {value};
   return allocate(node);
}

NodeId ASTArena::allocateUnlessStmt(NodeId statement, NodeId expression, size_t line) {
   Node node {StmtType::unlessStmt, line};
   node.unlessStmt = {statement, expression};
   return allocate(node);
}

NodeId ASTArena::allocateDoStmt(NodeId statement, size_t line) {
   Node node {StmtType::doStmt, line};
   node.doStmt = {statement};
   return allocate(node);
}

NodeId ASTArena::allocateImportStmt(const std::vector<NodeId> &values, const std::string &file, NodeId as, size_t line) {
   Node node {StmtType::importStmt, line};
   node.importStmt = {pushVector(values), pushString(file), as};
   return allocate(node);
}

NodeId ASTArena::allocateAssignment(NodeId left, NodeId right, TokenType op, size_t line) {
   Node node {StmtType::assignment, line};
   node.assignment = {left, right, op};
   return allocate(node);
}

NodeId ASTArena::allocateBinary(NodeId left, NodeId right, TokenType op, size_t line) {
   Node node {StmtType::binary, line};
   node.binary = {left, right, op};
   return allocate(node);
}

NodeId ASTArena::allocateUnary(NodeId value, TokenType op, size_t line) {
   Node node {StmtType::unary, line};
   node.unary = {value, op};
   return allocate(node);
}

NodeId ASTArena::allocatePropertyAccess(NodeId left, const std::vector<NodeId> &right, size_t line) {
   Node node {StmtType::property, line};
   node.property = {left, pushVector(right)};
   return allocate(node);
}

NodeId ASTArena::allocateFnCall(const std::string &identifier, const std::vector<NodeId> &args, size_t line) {
   Node node {StmtType::call, line};
   node.fnCall = {pushString(identifier), pushVector(args)};
   return allocate(node);
}

NodeId ASTArena::allocateEnumEntry(const std::string &identifier, NodeId value, const std::vector<NodeId> &args, const std::vector<NodeId> &argValues, size_t line) {
   Node node {StmtType::enumEntry, line};
   node.enumEntry = {pushString(identifier), value, pushVector(args), pushVector(argValues)};
   return allocate(node);
}

NodeId ASTArena::allocateIdentifier(const std::string &string, size_t line) {
   Node node {StmtType::identifier, line};
   node.identifier = {pushString(string)};
   return allocate(node);
}

NodeId ASTArena::allocateNumber(long double number, size_t line) {
   Node node {StmtType::number, line};
   node.number = {number};
   return allocate(node);
}

NodeId ASTArena::allocateString(const std::string &string, size_t line) {
   Node node {StmtType::string, line};
   node.string = {pushString(string)};
   return allocate(node);
}

NodeId ASTArena::allocateArray(const std::vector<NodeId> &values, size_t line) {
   Node node {StmtType::array, line};
   node.array = {pushVector(values)};
   return allocate(node);
}

NodeId ASTArena::allocateNull(size_t line) {
   Node node {StmtType::null, line};
   node.null = {};
   return allocate(node);
}

NodeId ASTArena::allocateProgram(const std::vector<NodeId> &nodes, size_t line) {
   Node node {StmtType::program, line};
   node.program = {pushVector(nodes)};
   return allocate(node);
}
