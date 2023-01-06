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

  func constexpr Print() -> const char* {
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

  func constexpr inline IsEnd() -> bool;
};

class Scanner {
  const char* start;
  const char* curr;
  u32 line;
  u32 row;

 public:
  Scanner() noexcept;
  func Init(const char* src) -> void;
  func ScanToken() -> Token;

 private:
  func inline Match(char expected) -> bool;
  func constexpr inline IsEnd() const -> const bool;
  func inline Pop() -> char;
  func inline MakeToken(Token::Lexeme type) const -> const Token;
  func constexpr inline Peek() const -> const char;
  func constexpr inline PeekNext() const -> const char;
  func SkipWhitespace() -> void;
  func CheckKeyword(u32 start, u32 length, const char* rest,
                    Token::Lexeme possible) const -> const Token::Lexeme;
  func StringToken() -> const Token;
  func NumberToken() -> const Token;
  func IdentifierToken() const -> const Token;
  func IdentifierType() const -> const Token::Lexeme;
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
  func ErrorAtCurr(const char* message) -> void;
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
func static Number(Compiler* compiler) -> void;
func static Parenthesis(Compiler* compiler) -> void;
func static Unary(Compiler* compiler) -> void;
func static Binary(Compiler* compiler) -> void;
func static Literal(Compiler* compiler) -> void;
func static String(Compiler* compiler) -> void;
}  // namespace Grammar

class Compiler {
 public:
  Compiler() noexcept;
  func Init(const char* src, Chunk* chunk, Arena<char>* string_pool) -> void;
  func Advance() -> void;
  func Consume(Token::Lexeme type, const char* message) -> void;
  func Compile() -> bool;
  func Expression() -> void;
  func EndCompilation() -> void;
  func GetPrecedence(Precedence prec) -> void;
  func GetParseRule(Token::Lexeme lexeme) -> ParseRule*;

  func Emit(u8 byte) -> void;
  func Emit(OpCode opcode) -> void;

  template <typename T, typename... Types>
  func Emit(T byte, Types... bytes) -> void {
    this->Emit(byte);
    if constexpr (sizeof...(bytes) != 0) this->Emit(bytes...);
  }

  func friend Grammar::Number(Compiler* compiler) -> void;
  func friend Grammar::Parenthesis(Compiler* compiler) -> void;
  func friend Grammar::Unary(Compiler* compiler) -> void;
  func friend Grammar::Binary(Compiler* compiler) -> void;
  func friend Grammar::Literal(Compiler* compiler) -> void;
  func friend Grammar::String(Compiler* compiler) -> void;

 private:
  Scanner* scanner;
  Parser* parser;
  Chunk* chunk = nullptr;
  Arena<char>* string_pool = nullptr;

  static std::unordered_map<Token::Lexeme, ParseRule> PARSE_RULES;
};

#undef VM_LEXEME_TYPE
