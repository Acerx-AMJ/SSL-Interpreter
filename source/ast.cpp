#include "ast.hpp"

ASTArena::ASTArena() {
   nodes.push_back({});
}

StringId ASTArena::pushString(const std::string &string) {
   StringId index = strings.size();
   strings.push_back(string);
   return index;
}

std::string_view ASTArena::getString(StringId id) const {
   return strings[id];
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

NodeId ASTArena::allocateBinary(NodeId left, NodeId right, TokenType op, size_t line) {
   Node node {StmtType::binary, line};
   node.binary = {left, right, op};
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

NodeId ASTArena::allocateNull(size_t line) {
   Node node {StmtType::null, line};
   node.null = {};
   return allocate(node);
}

NodeId ASTArena::allocateProgram(const std::vector<NodeId> &nodes, size_t line) {
   Node node {StmtType::program, line};
   node.program = {NodeList{children.size(), nodes.size()}};

   for (size_t i = 0; i < nodes.size(); ++i) {
      children.push_back(nodes[i]);
   }
   return allocate(node);
}
