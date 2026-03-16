#ifndef SSL_ENVIRONMENT_HPP
#define SSL_ENVIRONMENT_HPP

#include "values.hpp"

struct Environment {
   Environment(Environment *parent);
   Environment();

   void declare(const std::string &identifier, ValueId value, size_t line);
   void assign(const std::string &identifier, ValueId value, size_t line);
   bool exists(const std::string &identifier);
   ValueId get(const std::string &identifier, size_t line);
   Environment &resolve(const std::string &identifier, size_t line);

   // Members

   Environment *parent;
   std::unordered_map<std::string, ValueId> variables;
};

#endif
