#include "chunk.hpp"
#include <cstdio>

Chunk::Chunk() {
   // TODO: initialize vector capacities
}

void Chunk::disassemble() {
   for (size_t i = 0; i < code.size(); ++i) {
      printf("%04lu line %-4lu %s", i, lines[i], getOpcodeName(code[i]));

      switch (code[i]) {
      case OP_CONSTANT:
         i += 1;
         printf(" %d %lF", code[i], constants[code[i]]);
      }
      putchar('\n');
   }
}

void Chunk::write(uint8_t byte, size_t line) {
   code.push_back(byte);
   lines.push_back(line);
}

size_t Chunk::addConstant(Value value) {
   constants.push_back(value);
   return constants.size() - 1;
}
