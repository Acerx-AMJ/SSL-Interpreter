#include "input.hpp"
#include "error.hpp"
#include <filesystem>
#include <fstream>
#include <iostream>

void getInputOrReadFile(std::string &input) {
   std::error_code ec;
   if (!std::filesystem::exists(input, ec) || !std::filesystem::is_regular_file(input, ec)) {
      return;
   }

   std::ifstream file (input);
   if (!file) {
      raiseErrorNoLine("Could not open file '%s'.", input.c_str());
   }

   std::string temp;
   input.clear();

   while (std::getline(file, temp)) {
      input += temp + '\n';
   }
   return;
}
