#include "error.hpp"
#include <cstdarg>
#include <cstdio>
#include <cstdlib>
#include <sstream>

static const std::string *code = nullptr;

void setProgramCode(const std::string *code) {
   ::code = code;
}

void printSurroundingLines(size_t line) {
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
      printf("%-5lu %s\n", line, currentText.c_str());

      #ifdef unix
      printf("\e[91m");
      #endif

      size_t startingSpaces = 0;
      size_t endingSpaces = currentText.size() - 1;

      while (isspace(currentText[startingSpaces])) ++startingSpaces;
      while (isspace(currentText[endingSpaces]))   --endingSpaces;

      printf("%-5s %*s", std::string(std::to_string(line).size(), ' ').c_str(), (int)startingSpaces, "");
      printf("%s", std::string(endingSpaces + 1 - startingSpaces, '^').c_str());

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
}

void raiseError(size_t line, const char *error, ...) {
   if (!code) {
      exit(EXIT_FAILURE);
   }
   
   printSurroundingLines(line);

   va_list args;
   va_start(args, error);
   raiseErrorVargs(error, args);
   va_end(args);
}

void raiseErrorNoLine(const char *error, ...) {
   va_list args;
   va_start(args, error);
   raiseErrorVargs(error, args);
   va_end(args);
}

void raiseErrorVargs(const char *error, va_list args) {
   #ifdef unix
   printf("\e[91m");
   #endif

   printf("Program exited due to the following error:\n");

   #ifdef unix
   printf("\e[0m");
   #endif

   vprintf(error, args);
   putchar('\n');
   exit(EXIT_FAILURE);
}
