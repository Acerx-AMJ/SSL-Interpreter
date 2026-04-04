#ifndef SSL_CHUNK_HPP
#define SSL_CHUNK_HPP

#include "values.hpp"
#include <cstdint>
#include <vector>

enum Opcode: uint8_t {
   OP_CONSTANT,
   OP_ADD,
   OP_SUBTRACT,
   OP_MULTIPLY,
   OP_DIVIDE,
   OP_NEGATE,
   OP_RETURN,
};

constexpr const char *opcodeNames[] {
   "Constant", "Add", "Subtract", "Multiply", "Divide", "Negate", "Return"
};

constexpr const char *getOpcodeName(uint8_t op) {
   return opcodeNames[op];
}

struct Chunk {
   Chunk();

   void disassemble();
   void write(uint8_t byte, size_t line);
   size_t addConstant(Value value);

   // Members

   std::vector<uint8_t> code;
   std::vector<size_t> lines;
   std::vector<Value> constants;
};

#endif
