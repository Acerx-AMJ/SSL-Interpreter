#include "parser.hpp"

Parser::Parser(std::vector<Token> &tokens)
   : tokens(tokens) {}

Program &Parser::parse() {
   while (!is(TokenType::eof)) {
      NodeId stmt = parseStmt();
      arena.children.push_back(stmt);
      program.nodes.size += 1;
   }
   return program;
}

// parsing

NodeId Parser::parseStmt() {
   return parseExpr();
}

NodeId Parser::parseExpr() {
   return parseAdditiveExpr();
}

NodeId Parser::parseAdditiveExpr() {
   NodeId left = parseMultiplicativeExpr();
   while (is(TokenType::plus) || is(TokenType::minus)) {
      TokenType op = current().type;
      advance();

      NodeId right = parseMultiplicativeExpr();
      left = arena.allocateBinary(left, right, op, line());
   }
   return left;
}

NodeId Parser::parseMultiplicativeExpr() {
   NodeId left = parseExponentiativeExpr();
   while (is(TokenType::star) || is(TokenType::slash) || is(TokenType::mod)) {
      TokenType op = current().type;
      advance();

      NodeId right = parseExponentiativeExpr();
      left = arena.allocateBinary(left, right, op, line());
   }
   return left;
}

NodeId Parser::parseExponentiativeExpr() {
   NodeId left = parsePrimaryExpr();
   if (is(TokenType::pow)) {
      TokenType op = current().type;
      advance();

      NodeId right = parseExponentiativeExpr();
      left = arena.allocateBinary(left, right, op, line());
   }
   return left;
}

NodeId Parser::parsePrimaryExpr() {
   // numbers
   if (is(TokenType::number)) {
      long double number = 0.0;

      try {
         number = std::stold(current().lexeme);
      } catch (...) {
         raiseError(line(), "Failed to convert string '%s' to number. It might be too large, "
                            "too small or invalid.", current().lexeme.c_str());
      }
      advance();
      return arena.allocateNumber(number, line());
   }
   // strings
   else if (is(TokenType::string)) {
      Token &token = current();
      advance();
      return arena.allocateString(token.lexeme, line());
   }
   // unexpected expression
   else {
      raiseError(line(), "Expected primary expression, got '%s' instead.",
         getTokenTypeAsString(current().type));
   }
}

// utility

void Parser::advance() {
   if (index + 1 < tokens.size()) {
      index += 1;
   }
}

bool Parser::is(TokenType type) const {
   return index < tokens.size() && tokens[index].type == type;
}

Token &Parser::current() {
   return tokens[index];
}

size_t Parser::line() {
   if (index == 0) {
      return tokens[index].line;
   }
   return tokens[index - 1].line;
}
