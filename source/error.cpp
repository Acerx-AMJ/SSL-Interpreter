#include "error.hpp"
#include <cstdarg>
#include <cstdio>
#include <cstdlib>

void raiseErrorNoLine(const char *error, ...) {
   va_list args;
   va_start(args, error);
   vprintf(error, args);
   putchar('\n');
   va_end(args);
   exit(EXIT_FAILURE);
}
