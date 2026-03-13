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
