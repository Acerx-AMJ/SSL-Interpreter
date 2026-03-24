#ifndef SSL_ERROR_HPP
#define SSL_ERROR_HPP

#include <string>

void setProgramCode(const std::string *code);
void printSurroundingLines(size_t line);

[[noreturn]] void raiseError(size_t lines, const char *error, ...);
[[noreturn]] void raiseErrorNoLine(const char *error, ...);
[[noreturn]] void raiseErrorVargs(const char *error, va_list args);

#endif
