#pragma once

#include <unordered_map>

#include "arena.h"
#include "chunk.h"
#include "common.h"

#define VM_LEXEME_TYPE                                                        \
  X(Eof)                                                                      \
  X(Error)                                                                    \
  X(Comment)                                                                  \
  X(LeftParens)                                                               \
  X(RightParens)                                                              \
  X(LeftBrace)                                                                \
  X(RightBrace)                                                               \
  X(Comma)                                                                    \
  X(Dot)                                                                      \
  X(Minus)                                                                    \
  X(Plus)                                                                     \
  X(Semicolon)                                                                \
  X(Colon)                                                                    \
  X(Slash)                                                                    \
  X(Star)                                                                     \
  X(Bang) X(BangEqual) X(Equal) X(EqualEqual) X(Greater) X(GreaterEqual)      \
      X(Less) X(LessEqual) X(Identifier) X(String) X(Number) X(And) X(Else)   \
          X(False) X(For) X(Function) X(If) X(Or) X(Return) X(Struct) X(True) \
              X(Var) X(While)

struct Token {
  enum class Lexeme {
#define X(ID) ID,
    VM_LEXEME_TYPE
#undef X
  };

  Lexeme type;
  const char* start;
  u32 len;
  u32 line;

  Token() noexcept;
  Token(Lexeme type, const char* start, u32 len, u32 line) noexcept;
  Token(const char* error) noexcept;

  auto constexpr Print() -> const char* {
    switch (this->type) {
#define X(ID)      \
  case Lexeme::ID: \
    return #ID;
      VM_LEXEME_TYPE
#undef X
      default: {
        return "Eof";
      }
    }
  }

  auto constexpr inline IsEnd() -> bool;
};

class Scanner {
  const char* start;
  const char* curr;
  u32 line;
  u32 row;

 public:
  Scanner() noexcept;
  auto Init(const char* src) -> void;
  auto ScanToken() -> Token;

 private:
  auto inline Match(char expected) -> bool;
  auto constexpr inline IsEnd() const -> const bool;
  auto inline Pop() -> char;
  auto inline MakeToken(Token::Lexeme type) const -> const Token;
  auto constexpr inline Peek() const -> const char;
  auto constexpr inline PeekNext() const -> const char;
  auto SkipWhitespace() -> void;
  auto CheckKeyword(u32 start, u32 length, const char* rest,
                    Token::Lexeme possible) const -> const Token::Lexeme;
  auto StringToken() -> const Token;
  auto NumberToken() -> const Token;
  auto IdentifierToken() const -> const Token;
  auto IdentifierType() const -> const Token::Lexeme;
};

struct Parser {
  Token curr;
  Token prev;

  union {
    struct {
      u64 error : 1;
      u64 panic : 1;
    } state;
    u64 value = 0;
  };

  Parser() noexcept;
  auto ErrorAtCurr(const char* message) -> void;
};

enum class Precedence : u8 {
  None,
  Assignment,
  Or,
  And,
  Equality,
  Comparison,
  Term,
  Factor,
  Unary,
  Invoke,
  Primary
};

class Compiler;
using ParseFunction = void (*)(Compiler*);

struct ParseRule {
  ParseFunction prefix;
  ParseFunction infix;
  Precedence precedence;

  ParseRule() noexcept {
    this->prefix = nullptr;
    this->infix = nullptr;
    this->precedence = Precedence::None;
  }

  ParseRule(ParseFunction prefix, ParseFunction infix,
            Precedence precedence) noexcept
      : prefix{prefix}, infix{infix}, precedence{precedence} {}
};

// i would make these part of Compiler, but you can't take pointers
// to member functions to build the parser table
namespace Grammar {
auto static Number(Compiler* compiler) -> void;
auto static Parenthesis(Compiler* compiler) -> void;
auto static Unary(Compiler* compiler) -> void;
auto static Binary(Compiler* compiler) -> void;
auto static Literal(Compiler* compiler) -> void;
auto static String(Compiler* compiler) -> void;
}  // namespace Grammar

class Compiler {
 public:
  Compiler() noexcept;
  auto Init(const char* src, Chunk* chunk, Arena<char>* string_pool) -> void;
  auto Advance() -> void;
  auto Consume(Token::Lexeme type, const char* message) -> void;
  auto Compile() -> bool;
  auto Expression() -> void;
  auto EndCompilation() -> void;
  auto GetPrecedence(Precedence prec) -> void;
  auto GetParseRule(Token::Lexeme lexeme) -> ParseRule*;

  auto Emit(u8 byte) -> void;
  auto Emit(OpCode opcode) -> void;

  template <typename T, typename... Types>
  auto Emit(T byte, Types... bytes) -> void {
    this->Emit(byte);
    if constexpr (sizeof...(bytes) != 0) this->Emit(bytes...);
  }

  auto friend Grammar::Number(Compiler* compiler) -> void;
  auto friend Grammar::Parenthesis(Compiler* compiler) -> void;
  auto friend Grammar::Unary(Compiler* compiler) -> void;
  auto friend Grammar::Binary(Compiler* compiler) -> void;
  auto friend Grammar::Literal(Compiler* compiler) -> void;
  auto friend Grammar::String(Compiler* compiler) -> void;

 private:
  Scanner* scanner;
  Parser* parser;
  Chunk* chunk = nullptr;
  Arena<char>* string_pool = nullptr;

  static std::unordered_map<Token::Lexeme, ParseRule> PARSE_RULES;
};

#undef VM_LEXEME_TYPE
