#ifndef SSL_TOKENS_HPP
#define SSL_TOKENS_HPP

#include <string>
#include <unordered_map>

enum class TokenType: char {
   eof, keyword, identifier, number, string,
   increment, decrement, equals, plusEquals, minusEquals, starEquals, slashEquals, modEquals, powEquals,
   plus, minus, star, slash, mod, pow,
   divisible, equalsEquals, notEquals, bigger, biggerEquals, smaller, smallerEquals,
   lparen, rparen, lbracket, rbracket, lbrace, rbrace, comma, reference, dot,
};

enum class KeywordType: char {
   none, kand, kor, knot,
   kmut, kcon, kif, kelif, kelse, kend, kmatch, kcase,
   kenum, kfn, klambda, kstruct,
   kfor, kin, kto, kuntil, kloop, kwhile, kdo,
   kcontinue, kbreak, kreturn, kunless,
   kpub, kprv, kimport, kall, kfrom, kas,
};

constexpr const char *tokenTypeStrings[] {
   "EOF", "Keyword", "Identifier", "Number", "String",
   "++", "--", "=", "+=", "-=", "*=", "/=", "%=", "**=",
   "+", "-", "*", "/", "%", "**",
   "%%", "==", "!=", ">", ">=", "<", "<=",
   "(", ")", "{", "}", "[", "]", ",", "&", "."
};

constexpr const char *keywordTypesStrings[] {
   "none", "and", "or", "not",
   "mut", "con", "if", "elif", "else", "end", "match", "case",
   "enum", "fn", "lambda", "struct",
   "for", "in", "to", "until", "loop", "while", "do",
   "continue", "break", "return", "unless",
   "pub", "prv", "import", "all", "from", "as",
};

constexpr const char *getTokenTypeAsString(TokenType type) {
   return tokenTypeStrings[(size_t)type];
}

constexpr const char *getKeywordTypeAsString(KeywordType type) {
   return keywordTypesStrings[(size_t)type];
}

struct Token {
   TokenType type;
   KeywordType keywordType;
   std::string lexeme;
   size_t line;
};

static inline constexpr size_t maxOperatorSize = 3;
static inline const std::unordered_map<std::string_view, TokenType> operators {
   {"++",  TokenType::increment},
   {"--",  TokenType::decrement},
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

static inline const std::unordered_map<std::string_view, KeywordType> keywords {
   {"and",      KeywordType::kand},
   {"or",       KeywordType::kor},
   {"not",      KeywordType::knot},
   {"mut",      KeywordType::kmut},
   {"con",      KeywordType::kcon},
   {"if",       KeywordType::kif},
   {"elif",     KeywordType::kelif},
   {"else",     KeywordType::kelse},
   {"end",      KeywordType::kend},
   {"match",    KeywordType::kmatch},
   {"case",     KeywordType::kcase},
   {"enum",     KeywordType::kenum},
   {"fn",       KeywordType::kfn},
   {"lambda",   KeywordType::klambda},
   {"struct",   KeywordType::kstruct},
   {"for",      KeywordType::kfor},
   {"in",       KeywordType::kin},
   {"to",       KeywordType::kto},
   {"until",    KeywordType::kuntil},
   {"loop",     KeywordType::kloop},
   {"while",    KeywordType::kwhile},
   {"do",       KeywordType::kdo},
   {"continue", KeywordType::kcontinue},
   {"break",    KeywordType::kbreak},
   {"return",   KeywordType::kreturn},
   {"unless",   KeywordType::kunless},
   {"pub",      KeywordType::kpub},
   {"prv",      KeywordType::kprv},
   {"import",   KeywordType::kimport},
   {"all",      KeywordType::kall},
   {"from",     KeywordType::kfrom},
   {"as",       KeywordType::kas},
};

#endif
