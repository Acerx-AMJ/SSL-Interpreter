#include "compiler.hpp"
#include "error.hpp"

Compiler::Compiler(ASTArena &arena): arena(arena) {}

Chunk Compiler::compile(NodeList program) {
   for (size_t index = program.start; index < program.start + program.size; ++index) {
      NodeId node = arena.children[index];
      compileNode(node);
   }
   return chunk;
}

// Helpers

void Compiler::compileNode(NodeId node) {
   Node &n = arena.get(node);
   switch (n.type) {
   case StmtType::number:
      emitConstant(n.number.number, n.line);
      break;
   case StmtType::unary:
      compileNode(n.unary.value);
      emitByte(OP_NEGATE, n.line); // assume no bangs or references yet
      break;
   case StmtType::binary:
      compileNode(n.binary.left);
      compileNode(n.binary.right);

      switch (n.binary.op) {
      case TokenType::plus:  emitByte(OP_ADD, n.line);      break;
      case TokenType::minus: emitByte(OP_SUBTRACT, n.line); break;
      case TokenType::star:  emitByte(OP_MULTIPLY, n.line); break;
      case TokenType::slash: emitByte(OP_DIVIDE, n.line);   break;
      default:               raiseError(n.line, "BINARY OP NOT IMPLEMENTED!"); // TODO: fix
      }
      break;
   case StmtType::returnStmt:
      compileNode(n.returnStmt.value);
      emitByte(OP_RETURN, n.line);
      break;
   default: raiseError(n.line, "%s NOT IMPLEMENTED!", getStatementTypeAsString(n.type)); // TODO: fix
   }
}

void Compiler::emitByte(uint8_t byte, size_t line) {
   chunk.write(byte, line);
}

void Compiler::emitConstant(Value value, size_t line) {
   size_t index = chunk.addConstant(value);
   emitByte(OP_CONSTANT, line);
   emitByte(index, line);
}
