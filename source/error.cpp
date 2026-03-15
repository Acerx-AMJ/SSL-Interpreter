#include "error.hpp"
#include <cstdarg>
#include <cstdio>
#include <cstdlib>
#include <sstream>

static const std::string *code = nullptr;

void setProgramCode(const std::string *code) {
   ::code = code;
}

void raiseError(unsigned line, const char *error, ...) {
   if (!code) {
      exit(EXIT_FAILURE);
   }
   
   std::stringstream ss (*code);
   std::string previousText, currentText, nextText, temp;
   size_t previousLine = 0, nextLine = 0, index = 1;

   while (std::getline(ss, temp)) {
      if (index < line && !temp.empty()) {
         previousLine = index;
         previousText = temp;
      }
      else if (index == line) {
         currentText = temp;
      }
      else if (index > line && !temp.empty() && nextText.empty()) {
         nextText = temp;
         nextLine = index;
         break;
      }
      index += 1;
   }

   if (!previousText.empty()) {
      printf("%-5lu %s\n", previousLine, previousText.c_str());
      for (size_t i = previousLine; i < line - 1; ++i) {
         printf("%-5lu\n", i + 1);
      }
   }

   // Can the line ever be empty? I don't know
   if (!currentText.empty()) {
      printf("%-5u %s\n", line, currentText.c_str());
      printf("%-5s ", std::string(std::to_string(line).size(), ' ').c_str());

      #ifdef unix
      printf("\e[91m");
      #endif

      printf("%s", std::string(currentText.size(), '^').c_str());

      #ifdef unix
      printf("\e[0m");
      #endif

      putchar('\n');
   }

   if (!nextText.empty()) {
      for (size_t i = line; i < nextLine - 1; ++i) {
         printf("%-5lu\n", i + 1);
      }
      printf("%-5lu %s\n", nextLine, nextText.c_str());
   }

   #ifdef unix
   printf("\e[91m");
   #endif

   printf("Program exited due to the following error:\n");

   #ifdef unix
   printf("\e[0m");
   #endif

   va_list args;
   va_start(args, error);

   vprintf(error, args);
   putchar('\n');

   va_end(args);
   exit(EXIT_FAILURE);
}

void raiseErrorNoLine(const char *error, ...) {
   #ifdef unix
   printf("\e[91m");
   #endif

   printf("Program exited due to the following error:\n");

   #ifdef unix
   printf("\e[0m");
   #endif

   va_list args;
   va_start(args, error);

   vprintf(error, args);
   putchar('\n');

   va_end(args);
   exit(EXIT_FAILURE);
}
