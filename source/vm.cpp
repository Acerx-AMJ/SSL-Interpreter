#include "vm.hpp"
#include <cstdio>

VM::VM() {
   stack.resize(256);
}

void VM::execute(Chunk &chunk) {
   ip = 0;
   
   while (ip < chunk.code.size()) {
      uint8_t code = chunk.code[ip];
      ip += 1;

      switch (code) {
      case OP_CONSTANT:
         push(chunk.constants[chunk.code[ip]]); // push constant
         ip += 1;
         break;
      case OP_ADD:
         push(pop() + pop());
         break;
      case OP_SUBTRACT: {
         Value r = pop(), l = pop();
         push(l - r);
         break;
      } case OP_MULTIPLY:
         push(pop() * pop());
         break;
      case OP_DIVIDE: {
         Value r = pop(), l = pop();
         push(r == 0.0 ? 0.0 : l / r); // defined behaviour
         break;
      } case OP_NEGATE:
         push(-pop());
         break;
      case OP_RETURN:
         printf("%lF\n", pop());
         return;
      }
   }
}

void VM::push(Value value) {
   if (sp >= stack.size()) {
      stack.resize(stack.size() * 2);
   }
   stack[sp] = value;
   sp += 1;
}

Value VM::pop() {
   sp -= 1;
   return stack[sp];
}
