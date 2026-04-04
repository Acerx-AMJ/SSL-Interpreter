#ifndef SSL_VM_HPP
#define SSL_VM_HPP

#include "chunk.hpp"

struct VM {
   VM();

   void execute(Chunk &chunk);
   void push(Value value);
   Value pop();

   // Members
   
   size_t ip, sp = 0;
   std::vector<Value> stack;
};

#endif
