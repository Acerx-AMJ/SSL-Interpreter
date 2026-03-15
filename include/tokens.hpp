#ifndef SSL_TOKENS_HPP
#define SSL_TOKENS_HPP

#include <string>
#include <unordered_map>

enum class TokenType: char {
   eof, identifier, number, string,
   increment, decrement, colonEquals, equals, plusEquals, minusEquals, starEquals, slashEquals,
   modEquals, powEquals, plus, minus, star, slash, mod, pow,
   divisible, equalsEquals, notEquals, bigger, biggerEquals, smaller, smallerEquals,
   lparen, rparen, lbracket, rbracket, lbrace, rbrace, comma, reference, dot,
   none, kand, kor, knot,
   kif, kelif, kelse, kend, kmatch, kcase,
   kenum, kfn, klambda, kstruct,
   kfor, kin, kto, kuntil, kloop, kwhile, kdo,
   kcontinue, kbreak, kreturn, kunless,
   kpub, kprv, kimport, kall, kfrom, kas,
};

constexpr const char *tokenTypeStrings[] {
   "EOF", "Identifier", "Number", "String",
   "++", "--", ":=", "=", "+=", "-=", "*=", "/=", "%=", "**=",
   "+", "-", "*", "/", "%", "**",
   "%%", "==", "!=", ">", ">=", "<", "<=",
   "(", ")", "{", "}", "[", "]", ",", "&", ".",
   "none", "and", "or", "not",
   "if", "elif", "else", "end", "match", "case",
   "enum", "fn", "lambda", "struct",
   "for", "in", "to", "until", "loop", "while", "do",
   "continue", "break", "return", "unless",
   "pub", "prv", "import", "all", "from", "as",
};

constexpr const char *getTokenTypeAsString(TokenType type) {
   return tokenTypeStrings[(size_t)type];
}

struct Token {
   TokenType type;
   std::string lexeme;
   size_t line;
};

static inline constexpr size_t maxOperatorSize = 3;
static inline const std::unordered_map<std::string_view, TokenType> operators {
   {"++",  TokenType::increment},
   {"--",  TokenType::decrement},
   {":=",  TokenType::colonEquals},
   {"=",   TokenType::equals},
   {"+=",  TokenType::plusEquals},
   {"-=",  TokenType::minusEquals},
   {"*=",  TokenType::starEquals},
   {"/=",  TokenType::slashEquals},
   {"%=",  TokenType::modEquals},
   {"**=", TokenType::powEquals},
   {"+",   TokenType::plus},
   {"-",   TokenType::minus},
   {"*",   TokenType::star},
   {"/",   TokenType::slash},
   {"%",   TokenType::mod},
   {"**",  TokenType::pow},
   {"%%",  TokenType::divisible},
   {"==",  TokenType::equalsEquals},
   {"!=",  TokenType::notEquals},
   {">",   TokenType::bigger},
   {">=",  TokenType::biggerEquals},
   {"<",   TokenType::smaller},
   {"<=",  TokenType::smallerEquals},
   {"(",   TokenType::lparen},
   {")",   TokenType::rparen},
   {"{",   TokenType::lbracket},
   {"}",   TokenType::rbracket},
   {"[",   TokenType::lbrace},
   {"]",   TokenType::rbrace},
   {",",   TokenType::comma},
   {"&",   TokenType::reference},
   {".",   TokenType::dot},
};

static inline const std::unordered_map<std::string_view, TokenType> keywords {
   {"and",      TokenType::kand},
   {"or",       TokenType::kor},
   {"not",      TokenType::knot},
   {"if",       TokenType::kif},
   {"elif",     TokenType::kelif},
   {"else",     TokenType::kelse},
   {"end",      TokenType::kend},
   {"match",    TokenType::kmatch},
   {"case",     TokenType::kcase},
   {"enum",     TokenType::kenum},
   {"fn",       TokenType::kfn},
   {"lambda",   TokenType::klambda},
   {"struct",   TokenType::kstruct},
   {"for",      TokenType::kfor},
   {"in",       TokenType::kin},
   {"to",       TokenType::kto},
   {"until",    TokenType::kuntil},
   {"loop",     TokenType::kloop},
   {"while",    TokenType::kwhile},
   {"do",       TokenType::kdo},
   {"continue", TokenType::kcontinue},
   {"break",    TokenType::kbreak},
   {"return",   TokenType::kreturn},
   {"unless",   TokenType::kunless},
   {"pub",      TokenType::kpub},
   {"prv",      TokenType::kprv},
   {"import",   TokenType::kimport},
   {"all",      TokenType::kall},
   {"from",     TokenType::kfrom},
   {"as",       TokenType::kas},
};

#endif
