#ifndef SSL_ERROR_HPP
#define SSL_ERROR_HPP

#include <string>

void setProgramCode(const std::string *code);

[[noreturn]] void raiseError(unsigned line, const char *error, ...);
[[noreturn]] void raiseErrorNoLine(const char *error, ...);

#endif
