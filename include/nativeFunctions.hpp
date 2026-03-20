#ifndef SSL_NATIVE_FUNCTIONS_HPP
#define SSL_NATIVE_FUNCTIONS_HPP

#include "values.hpp"
#include <functional>

using NtFunc = std::function<ValueId(const std::vector<ValueId>&, struct Interpreter&, size_t)>;
const NtFunc &getNativeFunction(const std::string &identifier, size_t line);

#endif
