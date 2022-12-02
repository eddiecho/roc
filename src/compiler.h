#pragma once

#include "common.h"

#define VM_LEXEME_TYPE \
  X(Eof) \
  X(Error) \
  X(Comment) \
  X(LeftParens) X(RightParens) X(LeftBrace) X(RightBrace) \
  X(Comma) X(Dot) X(Minus) X(Plus) \
  X(Semicolon) X(Colon) X(Slash) X(Star) \
  X(Bang) X(BangEqual) X(Equal) X(EqualEqual) \
  X(Greater) X(GreaterEqual) X(Less) X(LessEqual) \
  X(Identifier) X(String) X(Number) \
  X(And) X(Else) X(False) X(For) \
  X(Function) X(If) X(Or) X(Return) \
  X(Struct) X(True) X(Var) X(While)

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
  explicit Token(const char* error);

  auto Type() -> const char* {
    switch (this->type) {
#define X(ID) case Lexeme::ID: return #ID;
    VM_LEXEME_TYPE
#undef X
    default: {
      return "Eof";
    }
    }
  }

  auto inline IsEnd() -> bool {
    return this->type == Lexeme::Eof;
  }
};

class Scanner {
  const char* start;
  const char* curr;
  u32 line;
  u32 row;

 public:
  auto Init(const char* src) -> void;
  auto ScanToken() -> Token;

 private:
  auto inline Match(char expected) -> bool;
  auto constexpr inline IsEnd() -> bool;
  auto inline Advance() -> char;
  auto inline MakeToken(Token::Lexeme type) -> Token;
  auto constexpr inline Peek() -> char;
  auto constexpr inline PeekNext() -> char;
  auto SkipWhitespace() -> void;
  auto CheckKeyword(u32 start, u32 length, const char* rest, Token::Lexeme possible) -> Token::Lexeme;
  auto StringToken() -> Token;
  auto NumberToken() -> Token;
  auto IdentifierToken() -> Token;
  auto IdentifierType() -> Token::Lexeme;
};

struct Compiler {
  Scanner* scanner;

  auto Compile(const char* src) -> void;
};

#undef VM_LEXEME_TYPE
