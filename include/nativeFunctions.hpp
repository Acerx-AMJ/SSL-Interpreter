#ifndef SSL_NATIVE_FUNCTIONS_HPP
#define SSL_NATIVE_FUNCTIONS_HPP

#include <cstdint>
#include <vector>

using ValueId = uint32_t;
using NtFunc = ValueId(*)(const std::vector<ValueId>&, struct Interpreter&, size_t);

#define NTFUNC(identifier) ValueId ntf##identifier(const std::vector<ValueId> &args, Interpreter &interpreter, size_t line)

// print utility
NTFUNC(print);
NTFUNC(println);
NTFUNC(printf);
NTFUNC(printfln);
NTFUNC(format);

#endif
