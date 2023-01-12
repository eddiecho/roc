#pragma once

#include <unordered_map>

#include "chunk.h"
#include "common.h"
#include "dynamic_array.h"
#include "string_pool.h"

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

  fnc constexpr Print() const -> const char* {
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
};

class Scanner {
 public:
  Scanner() noexcept;
  fnc Init(const char* src) -> void;
  fnc ScanToken() -> Token;

 private:
  fnc inline Match(char expected) -> bool;
  fnc constexpr inline IsEnd() const -> const bool;
  fnc inline Pop() -> char;
  fnc inline MakeToken(Token::Lexeme type) const -> const Token;
  fnc constexpr inline Peek() const -> const char;
  fnc constexpr inline PeekNext() const -> const char;
  fnc SkipWhitespace() -> void;
  fnc CheckKeyword(u32 start, u32 length, const char* rest,
                    Token::Lexeme possible) const -> const Token::Lexeme;
  fnc StringToken() -> const Token;
  fnc NumberToken() -> const Token;
  fnc IdentifierToken() const -> const Token;
  fnc IdentifierType() const -> const Token::Lexeme;

 private:
  const char* start;
  const char* curr;
  u32 line;
  u32 row;
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
using Parsefnction = void (*)(Compiler*);

struct ParseRule {
  Parsefnction prefix;
  Parsefnction infix;
  Precedence precedence;

  ParseRule() noexcept {
    this->prefix = nullptr;
    this->infix = nullptr;
    this->precedence = Precedence::None;
  }

  ParseRule(Parsefnction prefix, Parsefnction infix,
            Precedence precedence) noexcept
      : prefix{prefix}, infix{infix}, precedence{precedence} {}
};

// i would make these part of Compiler, but you can't take pointers
// to member fnctions to build the parser table
namespace Grammar {
fnc static Number(Compiler* compiler) -> void;
fnc static Parenthesis(Compiler* compiler) -> void;
fnc static Unary(Compiler* compiler) -> void;
fnc static Binary(Compiler* compiler) -> void;
fnc static Literal(Compiler* compiler) -> void;
fnc static String(Compiler* compiler) -> void;
}  // namespace Grammar

class Compiler {
 public:
  Compiler() noexcept;
  fnc Init(const char* src, Chunk* chunk, StringPool* string_pool) -> void;
  fnc Compile() -> bool;

 private:
  fnc Advance() -> void;
  fnc Consume(Token::Lexeme type, const char* message) -> void;
  fnc Expression() -> void;
  fnc EndCompilation() -> void;
  fnc ErrorAtCurr(const char* message) -> void;
  fnc GetPrecedence(Precedence prec) -> void;
  fnc GetParseRule(Token::Lexeme lexeme) -> ParseRule*;

  fnc Emit(u8 byte) -> void;
  fnc Emit(u8* bytes, u32 count) -> void;
  fnc Emit(OpCode opcode) -> void;

  fnc friend Grammar::Number(Compiler* compiler) -> void;
  fnc friend Grammar::Parenthesis(Compiler* compiler) -> void;
  fnc friend Grammar::Unary(Compiler* compiler) -> void;
  fnc friend Grammar::Binary(Compiler* compiler) -> void;
  fnc friend Grammar::Literal(Compiler* compiler) -> void;
  fnc friend Grammar::String(Compiler* compiler) -> void;

 private:
  Token curr;
  Token prev;

  union {
    struct {
      u64 error : 1;
      u64 panic : 1;
    } state;
    u64 value = 0;
  };

  Scanner scanner;
  Chunk* chunk = nullptr;
  StringPool* string_pool = nullptr;

  static std::unordered_map<Token::Lexeme, ParseRule> PARSE_RULES;
};

#undef VM_LEXEME_TYPE
