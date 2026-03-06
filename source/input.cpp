#include "input.hpp"
#include "error.hpp"
#include <filesystem>
#include <fstream>
#include <iostream>

void getInputOrReadFile(std::string &input) {
   try {
      if (!std::filesystem::exists(input) || !std::filesystem::is_regular_file(input)) {
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

   // most likely std::filesystem::is_regular_file failed due to the file name being
   // too long. this error handling sucks
   catch (...) {}
}
