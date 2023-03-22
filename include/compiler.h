#pragma once

#include <unordered_map>
#include "absl/container/flat_hash_map.h"

#include "chunk.h"
#include "common.h"
#include "dynamic_array.h"
#include "global_pool.h"
#include "object.h"
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
              X(Var) X(While) X(In)

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

  fnc IdentifiersEqual(Token other) const -> bool;
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
  fnc IdentifierToken() -> const Token;
  fnc IdentifierType() -> const Token::Lexeme;

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

struct Local {
  Token id;
  u32 depth;
};

#define M_MAX_LOCALS_COUNT 256
struct ScopedLocals {
  u32 locals_count = 0;
  Local locals[M_MAX_LOCALS_COUNT];
  ScopedLocals* prev = nullptr;
};

// For resolving upvalues in closures
struct LocalsList {
  ScopedLocals* head = nullptr;
};

class Compiler;
using ParseFunction = void (*)(Compiler*, bool);

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
// to member fnctions to build the parser table
namespace Grammar {
fnc static Number(Compiler* compiler, bool assign) -> void;
fnc static Parenthesis(Compiler* compiler, bool assign) -> void;
fnc static Unary(Compiler* compiler, bool assign) -> void;
fnc static Binary(Compiler* compiler, bool assign) -> void;
fnc static Literal(Compiler* compiler, bool assign) -> void;
fnc static String(Compiler* compiler, bool assign) -> void;
fnc static Variable(Compiler* compiler, bool assign) -> void;
fnc static AndOp(Compiler* compiler, bool assign) -> void;
fnc static OrOp(Compiler* compiler, bool assign) -> void;
fnc static InvokeOp(Compiler* compiler, bool assign) -> void;
}  // namespace Grammar

enum class CompileError {
  Syntax = 1,
};

using CompileResult = Result<Object*, CompileError>;
using ParseRuleMap = absl::flat_hash_map<Token::Lexeme, ParseRule>;

struct BlockState {
  union {
    u32 raw;
    struct {
      unsigned char has_captures: 1;
    };
  };
};

class Compiler {
 public:
  Compiler() noexcept;
  fnc Init(const char* src,
           StringPool* string_pool,
           GlobalPool* global_pool) -> void;
  fnc Compile() -> CompileResult;

 private:
  fnc Advance() -> void;
  fnc inline Match(Token::Lexeme type) -> bool;
  fnc Consume(Token::Lexeme type, const char* message) -> void;
  fnc Declaration() -> BlockState;
  fnc Expression(bool nested = false) -> void;
  fnc EndCompilation() -> void;
  fnc ErrorAtCurr(const char* message) -> void;
  fnc GetPrecedence(Precedence prec) -> void;
  fnc GetParseRule(Token::Lexeme lexeme) -> const ParseRule*;
  fnc Statement() -> void;
  fnc SyncOnError() -> void;
  fnc inline CurrentChunk() -> Chunk*;

  fnc FunctionDeclaration() -> BlockState;
  fnc VariableDeclaration() -> void;
  fnc LoadVariable(bool assignment) -> void;
  fnc Identifier(const char* err) -> u32;
  fnc BeginScope() -> void;
  fnc EndScope() -> void;
  fnc CodeBlock() -> BlockState;
  fnc AddGlobal(Token id) -> u64;
  fnc AddLocal(Token id) -> void;
  fnc AddUpvalue(Token id) -> void;
  fnc FindLocal(Token id, ScopedLocals* scope) -> Option<u64>;
  fnc FindLocal(Token id) -> Option<u64>;
  fnc FindUpvalue(Token id) -> Option<u64>;
  fnc FindGlobal(Token id) -> Option<u64>;

  fnc Jump(OpCode opcode) -> u32;
  fnc PatchJump(u64 jump_idx) -> void;
  fnc Loop(u64 loop_idx) -> void;

  fnc Emit(u8 byte) -> void;
  fnc Emit(u8* bytes, u32 count) -> void;
  fnc Emit(OpCode opcode) -> void;

  fnc friend Grammar::Number(Compiler* compiler, bool assign) -> void;
  fnc friend Grammar::Parenthesis(Compiler* compiler, bool assign) -> void;
  fnc friend Grammar::Unary(Compiler* compiler, bool assign) -> void;
  fnc friend Grammar::Binary(Compiler* compiler, bool assign) -> void;
  fnc friend Grammar::Literal(Compiler* compiler, bool assign) -> void;
  fnc friend Grammar::String(Compiler* compiler, bool assign) -> void;
  fnc friend Grammar::Variable(Compiler* compiler, bool assign) -> void;
  fnc friend Grammar::AndOp(Compiler* compiler, bool assign) -> void;
  fnc friend Grammar::OrOp(Compiler* compiler, bool assign) -> void;
  fnc friend Grammar::InvokeOp(Compiler* compiler, bool assign) -> void;

 private:
  constexpr static const u32 MAX_LOCALS_COUNT = M_MAX_LOCALS_COUNT;
  constexpr static const char* GLOBAL_FUNCTION_NAME = "GLOBAL_FUNCTION";
  constexpr static const u32 GLOBAL_FUNCTION_NAME_LEN = 15;
  const static ParseRuleMap PARSE_RULES;

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

  ChunkManager chunk_manager;
  Object::Function *curr_func = nullptr;
  ScopedLocals locals;
  LocalsList locals_list;
  u32 scope_depth = 0;

  Scanner scanner;
  StringPool* string_pool = nullptr;
  GlobalPool* global_pool = nullptr;
};

#undef VM_LEXEME_TYPE
#undef M_MAX_LOCALS_COUNT
