#include "error.hpp"
#include "input.hpp"

int main(int argc, char *argv[]) {
   if (argc != 2) {
      raiseErrorNoLine("Expected exactly 2 arguments, got %d instead.", argc);
   }

   std::string input = argv[1];
   getInputOrReadFile(input);
   printf("Input: '%s'\n", input.c_str());
   return 0;
}