#pragma once

#include <unordered_map>

#include "common.h"
#include "chunk.h"

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
  Scanner();
  auto Init(const char* src) -> void;
  auto ScanToken() -> Token;

 private:
  auto inline Match(char expected) -> bool;
  auto constexpr inline IsEnd() -> bool;
  auto inline Pop() -> char;
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

struct Compiler;

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

  friend Compiler;

  Parser();
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

typedef void (*ParseFunction)(Compiler*);
struct ParseRule {
  ParseFunction prefix;
  ParseFunction infix;
  Precedence precedence;

  ParseRule() {
    this->prefix = nullptr;
    this->infix = nullptr;
    this->precedence = Precedence::None;
  }

  ParseRule(ParseFunction prefix, ParseFunction infix, Precedence precedence) noexcept :
    prefix{prefix}, infix{infix}, precedence{precedence}
  {}
};

namespace Grammar {
  auto Number(Compiler* compiler) -> void;
  auto Parenthesis(Compiler* compiler) -> void;
  auto Unary(Compiler* compiler) -> void;
  auto Binary(Compiler* compiler) -> void;
}

struct Compiler {
  Scanner* scanner;
  Parser* parser;
  Chunk* chunk = nullptr;

 public:
  Compiler() noexcept;
  auto Init(const char* src, Chunk* chunk) -> void;
  auto Advance() -> void;
  auto Consume(Token::Lexeme type, const char* message) -> void;
  auto Compile() -> bool;
  auto Expression() -> void;
  auto EndCompilation() -> void;
  auto GetPrecedence(Precedence prec) -> void;
  auto GetParseRule(Token::Lexeme lexeme) -> ParseRule*;

  auto Emit(u8 byte) -> void;
  auto Emit(OpCode opcode) -> void;

  template <typename... Types>
  auto Emit(u8 byte, Types... bytes) -> void {
    this->Emit(byte);
    if constexpr (sizeof...(bytes) != 0) this->Emit(bytes...);
  }

 private:
  auto Constant(Value value) -> void;

  static std::unordered_map<Token::Lexeme, ParseRule> PARSE_RULES;
};



#undef VM_LEXEME_TYPE
