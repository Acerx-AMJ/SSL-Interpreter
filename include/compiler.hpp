#ifndef SSL_COMPILER_HPP
#define SSL_COMPILER_HPP

#include "ast.hpp"
#include "chunk.hpp"

struct Compiler {
   Compiler(ASTArena &arena);
   Chunk compile(NodeList program);

   // Helpers

   void compileNode(NodeId node);
   void emitByte(uint8_t byte, size_t line);
   void emitConstant(Value value, size_t line);

   // Members

   ASTArena &arena;
   Chunk chunk;
};

#endif
