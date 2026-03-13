#include "parser.hpp"
#include "error.hpp"

Parser::Parser(std::vector<Token> &tokens)
   : tokens(tokens) {}

Program &Parser::parse() {
   std::vector<NodeId> program;

   while (!is(TokenType::eof)) {
      NodeId stmt = parseStmt();
      program.push_back(stmt);
   }

   NodeId id = arena.allocateProgram(program, 1);
   return arena.get(id).program;
}

// parsing

NodeId Parser::parseStmt() {
   TokenType keyword = current().type;
   bool isPublic = true;
   bool wasPublicModifierSet = false;
   
   // advance on pub and prv keywords
   if (keyword == TokenType::kpub) {
      wasPublicModifierSet = true;
      advance();
   } else if (keyword == TokenType::kprv) {
      wasPublicModifierSet = true;
      isPublic = false;
      advance();
   }

   keyword = current().type;
   if (peek(TokenType::colonEquals)) {
      return parseVarDecl(isPublic);
   } else if (keyword == TokenType::kfn) {
      return parseFnDecl(isPublic);
   } else if (keyword == TokenType::kenum) {
      return parseEnumDecl(isPublic);
   }
   return parseExpr();
}

// statements

NodeId Parser::parseVarDecl(bool isPublic) {
   expect(StmtType::varDecl, TokenType::identifier);
   std::string identifier = current().lexeme;
   advance();
   advance(); // we know this from 'parseStmt'

   NodeId value = parseExpr();
   return arena.allocateVarDecl(isPublic, identifier, value, line());
}

NodeId Parser::parseFnDecl(bool isPublic) {
   advance();
   expect(StmtType::fnDecl, TokenType::identifier);
   std::string identifier = current().lexeme;

   advance();
   expect(StmtType::fnDecl, TokenType::lparen);
   std::vector<NodeId> parameters;

   do {
      advance();
      if (is(TokenType::rparen)) {
         break;
      }

      expect(StmtType::fnDecl, TokenType::identifier);

      NodeId identifier = parsePrimaryExpr();
      parameters.push_back(identifier);
   } while (is(TokenType::comma));

   expect(StmtType::fnDecl, TokenType::rparen);
   advance();

   NodeId program = parseProgram();
   return arena.allocateFnDecl(isPublic, identifier, program, parameters, line());
}

NodeId Parser::parseEnumDecl(bool isPublic) {
   advance();
   expect(StmtType::enumDecl, TokenType::identifier);

   std::string identifier = current().lexeme;
   std::vector<NodeId> entries;

   do {
      advance();
      if (current().type == TokenType::kend) {
         break;
      }

      expect(StmtType::enumDecl, TokenType::identifier);
      std::vector<NodeId> args, argValues;
      std::string identifier = current().lexeme;
      NodeId value = null;

      advance();
      if (current().type == TokenType::lparen) {
         do {
            advance();
            if (is(TokenType::rparen)) {
               break;
            }

            expect(StmtType::enumDecl, TokenType::identifier);
            NodeId identifier = parsePrimaryExpr();

            args.push_back(identifier);
            argValues.push_back(null);
         } while (is(TokenType::comma));

         expect(StmtType::enumDecl, TokenType::rparen);
         advance();
      }

      if (current().type == TokenType::equals) {
         advance();
         value = parseExpr();
      }
      
      NodeId entry = arena.allocateEnumEntry(identifier, value, args, argValues, line());
      entries.push_back(entry);
   } while (is(TokenType::comma));

   expect(StmtType::enumDecl, TokenType::kend);
   advance();
   return arena.allocateEnumDecl(isPublic, identifier, entries, line());
}

// expressions

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
   // identifiers
   else if (is(TokenType::identifier)) {
      Token &token = current();
      advance();
      return arena.allocateIdentifier(token.lexeme, line());
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

NodeId Parser::parseProgram() {
   std::vector<NodeId> nodes;
   size_t originalLine = line();
   
   while (current().type != TokenType::kend) {
      NodeId node = parseStmt();
      nodes.push_back(node);
   }

   advance();
   return arena.allocateProgram(nodes, originalLine);
}

// utility

void Parser::advance() {
   if (index + 1 < tokens.size()) {
      index += 1;
   }
}

void Parser::expect(StmtType type, TokenType expected) {
   if (!is(expected)) {
      raiseError(line(), "%s: expected %s, got %s instead.", getStatementTypeAsString(type),
         getTokenTypeAsString(expected), getTokenTypeAsString(current().type));
   }
}

bool Parser::is(TokenType type) const {
   return index < tokens.size() && tokens[index].type == type;
}

bool Parser::peek(TokenType type) const {
   return index + 1 < tokens.size() && tokens[index + 1].type == type;
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
