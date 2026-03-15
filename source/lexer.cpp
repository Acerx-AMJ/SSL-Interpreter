#include "lexer.hpp"
#include "error.hpp"

Lexer::Lexer(const std::string &code)
   : code(code) {}

std::vector<Token> &Lexer::lex() {
   for (char ch = current(); index < code.size(); ch = advance()) {
      if (isspace(ch)) {
         line += (ch == '\n');
         continue;
      }

      // single comments
      if (ch == '/' && peek() == '/') {
         line += 1;
         while (index < code.size() && ch != '\n') {
            ch = advance();
         }
      }
      // block comments
      else if (ch == '/' && peek() == '*') {
         size_t indent = 1;
         size_t originalLine = line;
         ch = advance();

         while (index < code.size() && indent > 0) {
            indent += (ch == '/' && peek() == '*');
            indent -= (ch == '*' && peek() == '/');

            line += (ch == '\n');
            ch = advance();
         }

         if (index >= code.size()) {
            raiseError(originalLine, "Unterminated block comment.");
         }
      }
      // numbers
      else if (isdigit(ch)) {
         std::string number;
         bool dot = false, lastDash = false;

         while (index < code.size()) {
            if (isdigit(ch)) {
               lastDash = false;
               number.push_back(ch);
            } else if (ch == '.') {
               if (dot || !isdigit(peek())) {
                  break;
               }
               dot = true;
               number.push_back(ch);
            } else if (ch != '\'') {
               break;
            }
            
            if (ch == '\'' || ch == '.') {
               if (lastDash) {
                  raiseError(line, "Expected number '%s' to not have two or more consecutive "
                                   "apostrophes or dots.", number.c_str());
               }
               lastDash = true;
            }
            ch = advance();
         }

         if (lastDash && code[index - 1] == '\'') {
            raiseError(line, "Expected number '%s' to not end with an apostrophe.", number.c_str());
         }

         pushToken(TokenType::number, number, line);
         index -= 1;
      }
      // strings
      else if (ch == '"' || ch == '\'') {
         std::string string;
         char end = ch;

         size_t originalLine = line;
         ch = advance();

         while (index < code.size() && ch != end) {
            line += (ch == '\n');
            if (ch == '\\') {
               ch = getEscapeCode(advance());
            }
            string.push_back(ch);
            ch = advance();
         }

         if (ch != end) {
            raiseError(originalLine, "Unterminated string.");
         }
         pushToken(TokenType::string, string, originalLine);
      }
      // identifiers
      else if (isalpha(ch) || ch == '_') {
         std::string identifier;

         while (index < code.size() && (isalnum(ch) || ch == '_')) {
            identifier.push_back(ch);
            ch = advance();
         }

         if (auto it = keywords.find(identifier); it != keywords.end()) {
            pushToken(it->second, identifier, line);
         } else {
            pushToken(TokenType::identifier, identifier, line);
         }
         index -= 1;
      }
      // operators
      else {
         std::string op;

         for (size_t i = 0; i < maxOperatorSize && index + i < code.size(); ++i) {
            op.push_back(code[index + i]);
         }

         size_t originalSize = op.size();
         for (size_t i = 0; i < originalSize; ++i) {
            if (auto oper = operators.find(op); oper != operators.end()) {
               pushToken(oper->second, op, line);
               break;
            }
            op.pop_back();
         }

         if (op.empty()) {
            raiseError(line, "Unexpected character: '{}'.", ch);
         }
         index += op.size() - 1;
      }
   }
   pushToken(TokenType::eof, "EOF", line);
   return tokens;
}

// helper functions

char Lexer::current() {
   if (index >= code.size()) {
      return 0;
   }
   return code[index];
}

char Lexer::peek() {
   if (index + 1 >= code.size()) {
      return 0;
   }
   return code[index + 1];
}

char Lexer::advance() {
   index += 1;
   return current();
}

char Lexer::getEscapeCode(char escape) {
   static const std::unordered_map<char, char> escapeCodeMap {
      {'a', '\a'}, {'b', '\b'}, {'t', '\t'}, {'n', '\n'}, {'v', '\v'}, {'f', '\f'},
      {'r', '\r'}, {'e', '\e'}, {'\\', '\\'}, {'\'', '\''}, {'"', '"'}
   };

   if (escapeCodeMap.find(escape) == escapeCodeMap.end()) {
      raiseError(line, "Unknown escape code '\\%c'.", escape);
   }
   return escapeCodeMap.at(escape);
}

void Lexer::pushToken(TokenType type, const std::string &lexeme, size_t line) {
   tokens.push_back(Token{type, lexeme, line});
}
