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

// statements

NodeId Parser::parseStmt() {
   TokenType keyword = current().type;
   bool isPublic = true;
   bool wasPublicModifierSet = false;
   
   // advance on pub and prv keywords
   if (keyword == TokenType::kpub) {
      wasPublicModifierSet = true;
      advance();
   }
   else if (keyword == TokenType::kprv) {
      wasPublicModifierSet = true;
      isPublic = false;
      advance();
   }

   keyword = current().type;
   if (peek(TokenType::colonEquals) || peek(TokenType::colonColon)) {
      return parseVarDecl(isPublic);
   }
   else if (keyword == TokenType::kfn) {
      return parseFnDecl(isPublic);
   }
   else if (keyword == TokenType::kenum) {
      return parseEnumDecl(isPublic);
   }
   else if (keyword == TokenType::kstruct) {
      return parseStructDecl(isPublic);
   }

   if (wasPublicModifierSet) {
      raiseError(line(), "pub/prv can only be used before variables, functions, enumerations and "
                         "structures, not %s.", getTokenTypeAsString(keyword));
   }

   if (keyword == TokenType::kif) {
      return parseIfStmt();
   }
   else if (keyword == TokenType::kmatch) {
      return parseMatchStmt();
   }
   else if (keyword == TokenType::kdo) {
      return parseDoWhileLoopOrNewScope();
   }
   else if (keyword == TokenType::kwhile) {
      return parseWhileLoop();
   }
   else if (keyword == TokenType::kloop) {
      return parseLoop();
   }
   else if (keyword == TokenType::kfor) {
      return parseForLoop();
   }
   else if (keyword == TokenType::kbreak) {
      return parseBreak();
   }
   else if (keyword == TokenType::kcontinue) {
      return parseContinue();
   }
   else if (keyword == TokenType::kreturn) {
      return parseReturn();
   }
   else if (keyword == TokenType::kimport) {
      return parseImport();
   }
   return parseExpr();
}

NodeId Parser::parseVarDecl(bool isPublic) {
   size_t originalLine = line();

   expect(StmtType::varDecl, TokenType::identifier);
   std::string identifier = current().lexeme;
   advance();

   bool isConstant = is(TokenType::colonColon);
   advance(); // we know this from 'parseStmt'

   NodeId value = parseExpr();
   return arena.allocateVarDecl(isPublic, isConstant, identifier, value, originalLine);
}

NodeId Parser::parseFnDecl(bool isPublic) {
   size_t originalLine = line();
   advance();

   expect(StmtType::fnDecl, TokenType::identifier);
   std::string module, identifier = current().lexeme;
   advance();

   if (is(TokenType::dot)) {
      advance();
      expect(StmtType::fnDecl, TokenType::identifier);

      std::swap(module, identifier);
      identifier = current().lexeme;
      advance();
   }

   expect(StmtType::fnDecl, TokenType::lparen);
   std::vector<NodeId> parameters;

   do {
      advance();
      if (is(TokenType::eof)) {
         raiseError(originalLine, "Unterminated function parameter list.");
      }

      if (is(TokenType::rparen)) {
         break;
      }

      bool isRef = false;
      if (is(TokenType::ref)) {
         advance();
         isRef = true;
      }

      expect(StmtType::fnDecl, TokenType::identifier);
      NodeId identifier = parsePrimaryExpr();

      arena.get(identifier).ref = isRef;
      parameters.push_back(identifier);
   } while (is(TokenType::comma));

   expect(StmtType::fnDecl, TokenType::rparen);
   advance();

   bool returnsRef = false;
   if (is(TokenType::arrow)) {
      advance();
      expect(StmtType::fnDecl, TokenType::ref);
      advance();
      returnsRef = true;
   }

   std::vector<NodeId> nodes;
   while (!is(TokenType::kend)) {
      if (is(TokenType::eof)) {
         raiseError(originalLine, "Unterminated function declaration.");
      }
      
      NodeId node = parseStmt();
      nodes.push_back(node);
   }

   advance();
   return arena.allocateFnDecl(isPublic, returnsRef, module, identifier, nodes, parameters, originalLine);
}

NodeId Parser::parseEnumDecl(bool isPublic) {
   size_t originalLine = line();
   advance();

   expect(StmtType::enumDecl, TokenType::identifier);
   std::string identifier = current().lexeme;
   std::vector<NodeId> entries;

   do {
      advance();
      size_t entryLine = line();
      
      if (is(TokenType::eof)) {
         raiseError(originalLine, "Unterminated enumeration declaration.");
      }

      if (is(TokenType::kend)) {
         break;
      }

      expect(StmtType::enumDecl, TokenType::identifier);
      std::vector<NodeId> args;
      std::string identifier = current().lexeme;
      NodeId value = null;

      advance();
      if (is(TokenType::lparen)) {
         do {
            advance();
            if (is(TokenType::rparen)) {
               break;
            }

            expect(StmtType::enumDecl, TokenType::identifier);
            NodeId identifier = parsePrimaryExpr();

            args.push_back(identifier);
         } while (is(TokenType::comma));

         expect(StmtType::enumDecl, TokenType::rparen);
         advance();
      }

      if (is(TokenType::equals)) {
         advance();
         value = parseExpr();
      }
      
      NodeId entry = arena.allocateEnumEntry(identifier, value, args, entryLine);
      entries.push_back(entry);
   } while (is(TokenType::comma));

   expect(StmtType::enumDecl, TokenType::kend);
   advance();
   return arena.allocateEnumDecl(isPublic, identifier, entries, originalLine);
}

NodeId Parser::parseStructDecl(bool isPublic) {
   size_t originalLine = line();
   advance();

   expect(StmtType::structDecl, TokenType::identifier);
   std::string identifier = current().lexeme;
   std::vector<NodeId> variables;

   do {
      advance();
      if (is(TokenType::eof)) {
         raiseError(originalLine, "Unterminated struct declaration.");
      }

      if (is(TokenType::kend)) {
         break;
      }

      expect(StmtType::structDecl, TokenType::identifier);

      NodeId identifier = parsePrimaryExpr();
      variables.push_back(identifier);
   } while (is(TokenType::comma));

   expect(StmtType::structDecl, TokenType::kend);
   advance();
   return arena.allocateStructDecl(isPublic, identifier, variables, originalLine);
}

NodeId Parser::parseIfStmt() {
   size_t originalLine = line();
   NodeId ifClause = parseIfClause();
   std::vector<NodeId> elifClauses;

   while (is(TokenType::kelif)) {
      NodeId elifClause = parseIfClause();
      elifClauses.push_back(elifClause);
   }

   NodeId elseClause = null;
   if (is(TokenType::kelse)) {
      elseClause = parseIfClause();
   }

   expect(StmtType::ifStmt, TokenType::kend);
   advance();
   return arena.allocateIfStmt(ifClause, elifClauses, elseClause, originalLine);
}

NodeId Parser::parseIfClause() {
   size_t originalLine = line();
   TokenType keyword = current().type;
   advance();

   std::vector<NodeId> nodes;
   NodeId expression = null;

   if (keyword == TokenType::kcase) {
      expression = parseRangeExpr();
   }
   else if (keyword != TokenType::kelse) {
      expression = parseExpr();
   }

   while (!is(TokenType::kend) && !is(TokenType::kelif) && !is(TokenType::kelse)
       && !is(TokenType::kcase)) {

      if (is(TokenType::eof)) {
         raiseError(originalLine, "Unterminated %s statement.", getTokenTypeAsString(keyword));
      }
      NodeId node = parseStmt();
      nodes.push_back(node);
   }
   return arena.allocateIfClause(keyword, expression, nodes, originalLine);
}

NodeId Parser::parseMatchStmt() {
   size_t original_line = line();
   advance();

   NodeId expression = null;
   std::vector<NodeId> cases;

   if (!is(TokenType::kcase) && !is(TokenType::kelse) && !is(TokenType::kend)) {
      expression = parseExpr();
   }

   while (is(TokenType::kcase)) {
      NodeId caseClause = parseIfClause();
      cases.push_back(caseClause);
   }

   NodeId elseClause = null;
   if (is(TokenType::kelse)) {
      elseClause = parseIfClause();
   }

   expect(StmtType::matchStmt, TokenType::kend);
   advance();
   return arena.allocateMatchStmt(expression, cases, elseClause, original_line);
}

NodeId Parser::parseForLoop() {
   size_t originalLine = line();
   advance();

   expect(StmtType::forLoop, TokenType::identifier);
   std::string identifier = current().lexeme;
   advance();

   std::string indexIdentifier;
   if (is(TokenType::comma)) {
      advance();
      expect(StmtType::forLoop, TokenType::identifier);
      indexIdentifier = current().lexeme;
      advance();
   }

   expect(StmtType::forLoop, TokenType::kin);
   advance();

   NodeId expression = parseRangeExpr();
   std::vector<NodeId> nodes;
   bool reversed = false;

   if (is(TokenType::kreversed)) {
      advance();
      reversed = true;
   }

   while (!is(TokenType::kend)) {
      if (is(TokenType::eof)) {
         raiseError(originalLine, "Unterminated for loop.");
      }

      NodeId node = parseStmt();
      nodes.push_back(node);
   }

   advance();
   return arena.allocateForLoop(reversed, identifier, indexIdentifier, expression, nodes, originalLine);
}

NodeId Parser::parseLoop() {
   size_t originalLine = line();
   std::vector<NodeId> nodes;
   advance();

   while (!is(TokenType::kend)) {
      if (is(TokenType::eof)) {
         raiseError(originalLine, "Unterminated loop.");
      }

      NodeId node = parseStmt();
      nodes.push_back(node);
   }

   advance();
   return arena.allocateLoop(nodes, originalLine);
}

NodeId Parser::parseWhileLoop() {
   size_t originalLine = line();
   advance();

   NodeId expression = parseExpr();
   std::vector<NodeId> nodes;

   while (!is(TokenType::kend)) {
      if (is(TokenType::eof)) {
         raiseError(originalLine, "Unterminated while loop.");
      }

      NodeId node = parseStmt();
      nodes.push_back(node);
   }

   advance();
   return arena.allocateWhileLoop(expression, nodes, originalLine);
}

NodeId Parser::parseDoWhileLoopOrNewScope() {
   size_t originalLine = line();
   std::vector<NodeId> nodes;
   advance();

   while (!is(TokenType::kend) && !is(TokenType::kwhile)) {
      if (is(TokenType::eof)) {
         raiseError(originalLine, "Unterminated do statement.");
      }

      NodeId node = parseStmt();
      nodes.push_back(node);
   }

   if (is(TokenType::kend)) {
      advance();
      return arena.allocateProgram(nodes, originalLine);
   }

   advance();
   NodeId expression = parseExpr();
   return arena.allocateDoWhileLoop(expression, nodes, originalLine);
}

NodeId Parser::parseBreak() {
   size_t originalLine = line();
   NodeId breakStmt = arena.allocateBreakStmt(originalLine);
   advance();
   return parseUnless(breakStmt);
}

NodeId Parser::parseContinue() {
   size_t originalLine = line();
   NodeId continueStmt = arena.allocateContinueStmt(originalLine);
   advance();
   return parseUnless(continueStmt);
}

NodeId Parser::parseReturn() {
   size_t originalLine = line();
   advance();

   NodeId expression = null;
   if (!is(TokenType::kend) && !is(TokenType::kunless)) {
      expression = parseExpr();
   }
   NodeId returnStmt = arena.allocateReturnStmt(expression, originalLine);
   return parseUnless(returnStmt);
}

NodeId Parser::parseUnless(NodeId statement) {
   if (!is(TokenType::kunless)) {
      return statement;
   }
   advance();
   NodeId expression = parseExpr();
   return arena.allocateUnlessStmt(statement, expression, arena.get(statement).line);
}

NodeId Parser::parseImport() {
   size_t originalLine = line();
   std::vector<NodeId> values;

   if (!peek(TokenType::kall)) {
      do {
         advance();
         if (is(TokenType::eof)) {
            raiseError(originalLine, "Unterminated import statement.");
         }

         if (is(TokenType::kfrom)) {
            break;
         }

         expect(StmtType::importStmt, TokenType::identifier);
         NodeId identifier = parsePrimaryExpr();
         values.push_back(identifier);
      } while (is(TokenType::comma));
   }
   else {
      advance();
      advance();
   }

   expect(StmtType::importStmt, TokenType::kfrom);
   advance();

   expect(StmtType::importStmt, TokenType::string);
   std::string file = current().lexeme;
   std::string as;
   advance();

   if (is(TokenType::kas)) {
      advance();
      expect(StmtType::importStmt, TokenType::identifier);
      as = current().lexeme;
      advance();
   }
   return arena.allocateImportStmt(values, file, as, originalLine);
}

// expressions

NodeId Parser::parseRangeExpr() {
   size_t originalLine = line();
   NodeId left = parseExpr();

   if (is(TokenType::kto) || is(TokenType::kuntil)) {
      bool inclusive = is(TokenType::kto);
      advance();
      NodeId right = parseExpr();
      left = arena.allocateRange(inclusive, left, right, originalLine);
   }
   return left;
}

NodeId Parser::parseExpr() {
   return parseAssignmentExpr();
}

NodeId Parser::parseAssignmentExpr() {
   size_t originalLine = line();
   NodeId left = parseNullCoalesceExpr();

   if (is(TokenType::equals) || is(TokenType::plusEquals) || is(TokenType::minusEquals)
    || is(TokenType::starEquals) || is(TokenType::slashEquals) || is(TokenType::modEquals)
    || is(TokenType::powEquals)) {
      TokenType op = current().type;
      advance();

      NodeId right = parseAssignmentExpr();
      left = arena.allocateAssignment(left, right, op, originalLine);
   }
   return left;
}

NodeId Parser::parseNullCoalesceExpr() {
   size_t originalLine = line();
   NodeId left = parseLogicalOrExpr();

   if (is(TokenType::nullOr)) {
      TokenType op = current().type;
      advance();

      NodeId right = parseNullCoalesceExpr();
      return arena.allocateBinary(left, right, op, originalLine);
   }
   return left;
}

NodeId Parser::parseLogicalOrExpr() {
   size_t originalLine = line();
   NodeId left = parseLogicalAndExpr();

   while (is(TokenType::kor)) {
      TokenType op = current().type;
      advance();

      NodeId right = parseLogicalAndExpr();
      left = arena.allocateBinary(left, right, op, originalLine);
   }
   return left;
}

NodeId Parser::parseLogicalAndExpr() {
   size_t originalLine = line();
   NodeId left = parseEqualityExpr();

   while (is(TokenType::kand)) {
      TokenType op = current().type;
      advance();

      NodeId right = parseEqualityExpr();
      left = arena.allocateBinary(left, right, op, originalLine);
   }
   return left;
}

NodeId Parser::parseEqualityExpr() {
   size_t originalLine = line();
   NodeId left = parseRelationalExpr();

   while (is(TokenType::equalsEquals) || is(TokenType::notEquals) || is(TokenType::divisible)) {
      TokenType op = current().type;
      advance();

      NodeId right = parseRelationalExpr();
      left = arena.allocateBinary(left, right, op, originalLine);
   }
   return left;
}

NodeId Parser::parseRelationalExpr() {
   size_t originalLine = line();
   NodeId left = parseAdditiveExpr();

   while (is(TokenType::bigger) || is(TokenType::biggerEquals) || is(TokenType::smaller)
       || is(TokenType::smallerEquals)) {
      TokenType op = current().type;
      advance();

      NodeId right = parseAdditiveExpr();
      left = arena.allocateBinary(left, right, op, originalLine);
   }
   return left;
}

NodeId Parser::parseAdditiveExpr() {
   size_t originalLine = line();
   NodeId left = parseMultiplicativeExpr();

   while (is(TokenType::plus) || is(TokenType::minus)) {
      TokenType op = current().type;
      advance();

      NodeId right = parseMultiplicativeExpr();
      left = arena.allocateBinary(left, right, op, originalLine);
   }
   return left;
}

NodeId Parser::parseMultiplicativeExpr() {
   size_t originalLine = line();
   NodeId left = parseExponentiativeExpr();

   while (is(TokenType::star) || is(TokenType::slash) || is(TokenType::mod)) {
      TokenType op = current().type;
      advance();

      NodeId right = parseExponentiativeExpr();
      left = arena.allocateBinary(left, right, op, originalLine);
   }
   return left;
}

NodeId Parser::parseExponentiativeExpr() {
   size_t originalLine = line();
   NodeId left = parseUnaryExpr();

   if (is(TokenType::pow)) {
      TokenType op = current().type;
      advance();

      NodeId right = parseExponentiativeExpr();
      left = arena.allocateBinary(left, right, op, originalLine);
   }
   return left;
}

NodeId Parser::parseUnaryExpr() {
   if (is(TokenType::plus) || is(TokenType::minus) || is(TokenType::knot) || is(TokenType::ref)) {
      size_t originalLine = line();
      TokenType op = current().type;

      advance();
      NodeId value = parseUnaryExpr();
      return arena.allocateUnary(value, op, originalLine);
   }
   return parsePostfixExpr();
}

NodeId Parser::parsePostfixExpr() {
   size_t originalLine = line();
   NodeId left = parsePrimaryExpr();

   while (true) {
      if (is(TokenType::dot) || is(TokenType::nullDot)) {
         bool nullChecked = is(TokenType::nullDot);
         advance();

         expect(StmtType::property, TokenType::identifier);
         std::string identifier = current().lexeme;
         advance();

         left = arena.allocatePropertyAccess(nullChecked, left, identifier, originalLine);
      }
      else if (is(TokenType::lbracket)) {
         do {
            advance();
            if (is(TokenType::eof)) {
               raiseError(originalLine, "Unterminated array subscript.");
            }

            if (is(TokenType::rbracket)) {
               break;
            }

            NodeId subscript = parseExpr();
            left = arena.allocateArraySubscript(left, subscript, originalLine);
            originalLine = line();
         } while (is(TokenType::comma));

         expect(StmtType::arraySubscript, TokenType::rbracket);
         advance();
      }
      else if (is(TokenType::lparen)) {
         std::vector<NodeId> arguments;

         do {
            advance();
            if (is(TokenType::eof)) {
               raiseError(originalLine, "Unterminated function call argument list.");
            }
            
            if (is(TokenType::rparen)) {
               break;
            }

            NodeId argument = parseExpr();
            arguments.push_back(argument);
         } while (is(TokenType::comma));

         expect(StmtType::call, TokenType::rparen);
         advance();

         left = arena.allocateFnCall(left, arguments, originalLine);
      }
      else {
         break;
      }
      originalLine = line();
   }
   return left;
}

NodeId Parser::parsePrimaryExpr() {
   if (is(TokenType::number)) {
      size_t originalLine = line();
      long double number = 0.0;

      try {
         number = std::stold(current().lexeme);
      }
      catch (...) {
         raiseError(originalLine, "Failed to convert string '%s' to number. It might be too large, "
                                  "too small or invalid.", current().lexeme.c_str());
      }
      advance();
      return arena.allocateNumber(number, originalLine);
   }
   else if (is(TokenType::identifier)) {
      Token &token = current();
      advance();
      return arena.allocateIdentifier(token.lexeme, token.line);
   }
   else if (is(TokenType::string)) {
      Token &token = current();
      advance();
      return arena.allocateString(token.lexeme, token.line);
   }
   else if (is(TokenType::lparen)) {
      size_t originalLine = line();
      advance();

      NodeId expression = parseExpr();
      expect(StmtType::program, TokenType::rparen);
      advance();
      return expression;
   }
   else if (is(TokenType::lbracket)) {
      size_t originalLine = line();
      std::vector<NodeId> values;

      do {
         advance();
         if (is(TokenType::eof)) {
            raiseError(originalLine, "Unterminated array.");
         }

         if (is(TokenType::rbracket)) {
            break;
         }

         NodeId value = parseExpr();
         values.push_back(value);
      } while (is(TokenType::comma));

      expect(StmtType::array, TokenType::rbracket);
      advance();
      return arena.allocateArray(values, originalLine);
   }
   else if (is(TokenType::klambda)) {
      size_t originalLine = line();
      advance();
      expect(StmtType::lambda, TokenType::lparen);
      std::vector<NodeId> parameters;

      do {
         advance();
         if (is(TokenType::eof)) {
            raiseError(originalLine, "Unterminated lambda parameter list.");
         }

         if (is(TokenType::rparen)) {
            break;
         }

         bool isRef = false;
         if (is(TokenType::ref)) {
            advance();
            isRef = true;
         }

         expect(StmtType::lambda, TokenType::identifier);
         NodeId identifier = parsePrimaryExpr();

         arena.get(identifier).ref = isRef;
         parameters.push_back(identifier);
      } while (is(TokenType::comma));

      std::vector<NodeId> nodes;
      expect(StmtType::lambda, TokenType::rparen);
      advance();

      bool returnsRef = false;
      if (is(TokenType::arrow)) {
         advance();
         expect(StmtType::lambda, TokenType::ref);
         advance();
         returnsRef = true;
      }

      while (!is(TokenType::kend)) {
         if (is(TokenType::eof)) {
            raiseError(originalLine, "Unterminated lambda declaration.");
         }
         
         NodeId node = parseStmt();
         nodes.push_back(node);
      }

      advance();
      return arena.allocateLambda(returnsRef, nodes, parameters, originalLine);
   }
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

void Parser::expect(StmtType type, TokenType expected) {
   if (!is(expected)) {
      raiseError(line(), "%s: expected %s, got %s instead.", getStatementTypeAsString(type),
         getTokenTypeAsString(expected), getTokenTypeAsString(current().type));
   }
}

bool Parser::is(TokenType type) const {
   return tokens[index].type == type;
}

bool Parser::peek(TokenType type) const {
   return index + 1 < tokens.size() && tokens[index + 1].type == type;
}

Token &Parser::current() {
   return tokens[index];
}

size_t Parser::line() {
   return tokens[index].line;
}
