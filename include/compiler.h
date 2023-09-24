#pragma once

#include <unordered_map>

#include "absl/container/flat_hash_map.h"
#include "chunk.h"
#include "common.h"
#include "dynamic_array.h"
#include "global_pool.h"
#include "object.h"
#include "string_pool.h"

#define VM_LEXEME_TYPE \
  X(Eof)               \
  X(Error)             \
  X(Comment)           \
  X(LeftParens)        \
  X(RightParens)       \
  X(LeftBrace)         \
  X(RightBrace)        \
  X(Comma)             \
  X(Dot)               \
  X(Minus)             \
  X(Plus)              \
  X(Semicolon)         \
  X(Colon)             \
  X(Slash)             \
  X(Star)              \
  X(Bang)              \
  X(BangEqual)         \
  X(Equal)             \
  X(EqualEqual)        \
  X(Greater)           \
  X(GreaterEqual)      \
  X(Less)              \
  X(LessEqual)         \
  X(Identifier)        \
  X(String)            \
  X(Number)            \
  X(And)               \
  X(Else)              \
  X(False)             \
  X(For)               \
  X(Function)          \
  X(If)                \
  X(Or)                \
  X(Return)            \
  X(Struct)            \
  X(True)              \
  X(Var)               \
  X(While)             \
  X(In)

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
  explicit Token(const char* error) noexcept;

  auto IdentifiersEqual(Token other) const -> bool;
  auto constexpr Print() const -> const char* {
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
  auto CheckKeyword(u32 start, u32 length, const char* rest, Token::Lexeme possible) const -> const Token::Lexeme;
  auto StringToken() -> const Token;
  auto NumberToken() -> const Token;
  auto IdentifierToken() -> const Token;
  auto IdentifierType() -> Token::Lexeme;

 private:
  const char* start;
  const char* curr;
  u32 line;
  u32 row;
};

enum class Precedence : u8 { None, Assignment, Or, And, Equality, Comparison, Term, Factor, Unary, Invoke, Primary };

struct Local {
  Token id;
  u32 depth;
  bool captured = false;
};

struct Upvalue {
  u8 index;
  bool local;
};

class Compiler;
class CompilerEngine;
using ParseFunction = void (*)(CompilerEngine*, bool);

struct ParseRule {
  ParseFunction prefix;
  ParseFunction infix;
  Precedence precedence;

  ParseRule() noexcept {
    this->prefix = nullptr;
    this->infix = nullptr;
    this->precedence = Precedence::None;
  }

  ParseRule(ParseFunction prefix, ParseFunction infix, Precedence precedence) noexcept
      : prefix{prefix}, infix{infix}, precedence{precedence} {}
};

// i would make these part of Compiler, but you can't take pointers
// to member autotions to build the parser table
namespace Grammar {
auto static Number(CompilerEngine* compiler, bool assign) -> void;
auto static Parenthesis(CompilerEngine* compiler, bool assign) -> void;
auto static Unary(CompilerEngine* compiler, bool assign) -> void;
auto static Binary(CompilerEngine* compiler, bool assign) -> void;
auto static Literal(CompilerEngine* compiler, bool assign) -> void;
auto static String(CompilerEngine* compiler, bool assign) -> void;
auto static Variable(CompilerEngine* compiler, bool assign) -> void;
auto static AndOp(CompilerEngine* compiler, bool assign) -> void;
auto static OrOp(CompilerEngine* compiler, bool assign) -> void;
auto static InvokeOp(CompilerEngine* compiler, bool assign) -> void;
}  // namespace Grammar

enum class CompileError {
  Syntax = 1,
};

using CompileResult = Result<Object*, CompileError>;
using ParseRuleMap = absl::flat_hash_map<Token::Lexeme, ParseRule>;

struct CompilerState {
  union {
    struct {
      u64 error : 1;
      u64 panic : 1;
      u64 has_captures : 1;
    };
    u64 value = 0;
  };

  auto Merge(CompilerState other) -> void;
};

class Compiler {
  friend CompilerEngine;

 public:
  Compiler() noexcept;
  auto Init(const char* src, StringPool* string_pool, GlobalPool* global_pool) -> void;
  auto Compile() -> CompileResult;

 private:
  constexpr static const char* GLOBAL_FUNCTION_NAME = "GLOBAL_FUNCTION";
  constexpr static const u32 GLOBAL_FUNCTION_NAME_LEN = 15;
  constexpr static const u32 MAX_LOCALS_COUNT = 256;
  const static ParseRuleMap PARSE_RULES;

 private:
  ChunkManager chunk_manager;
  Scanner scanner;
  StringPool* string_pool = nullptr;
  GlobalPool* global_pool = nullptr;
};

class CompilerEngine {
  friend Compiler;

 public:
  auto Init(Compiler* compiler) -> void;
  auto Init(Compiler* compiler, u32 name_length, const char* name) -> void;
  auto Compile() -> CompileResult;

 private:
  auto Advance() -> void;
  auto inline MatchAndAdvance(Token::Lexeme type) -> bool;
  auto Consume(Token::Lexeme type, const char* message) -> void;
  auto Declaration() -> void;
  auto Expression(bool nested = false) -> void;
  auto EndCompilation() -> void;
  auto ErrorAtToken(const char* message, Token id) -> void;
  auto ErrorAtCurr(const char* message) -> void;
  auto GetPrecedence(Precedence prec) -> void;
  auto GetParseRule(Token::Lexeme lexeme) -> const ParseRule*;
  auto Statement() -> void;
  auto SyncOnError() -> void;
  auto inline CurrentChunk() -> Chunk*;

  auto FunctionDeclaration() -> void;
  auto FunctionBody() -> void;
  auto VariableDeclaration() -> void;
  auto LoadVariable(bool assignment) -> void;
  auto Identifier(const char* err) -> u32;
  auto BeginScope() -> void;
  auto EndScope() -> void;
  auto CodeBlock() -> void;
  auto AddString(u32 length, const char* start) -> u32;
  auto AddGlobal(Token id) -> u64;
  auto AddLocal(Token id) -> void;
  auto AddUpvalue(u8 index, bool local) -> u32;
  auto FindLocal(Token id, Local* locals) -> Option<u64>;
  auto FindLocal(Token id) -> Option<u64>;
  auto FindUpvalue(Token id) -> Option<u64>;
  auto FindGlobal(Token id) -> Option<u64>;

  auto Jump(OpCode opcode) -> u32;
  auto PatchJump(u64 jump_idx) -> void;
  auto Loop(u64 loop_idx) -> void;

  auto Emit(u8 byte) -> void;
  auto Emit(u8* bytes, u32 count) -> void;
  auto Emit(OpCode opcode) -> void;

  auto friend Grammar::Number(CompilerEngine* compiler, bool assign) -> void;
  auto friend Grammar::Parenthesis(CompilerEngine* compiler, bool assign) -> void;
  auto friend Grammar::Unary(CompilerEngine* compiler, bool assign) -> void;
  auto friend Grammar::Binary(CompilerEngine* compiler, bool assign) -> void;
  auto friend Grammar::Literal(CompilerEngine* compiler, bool assign) -> void;
  auto friend Grammar::String(CompilerEngine* compiler, bool assign) -> void;
  auto friend Grammar::Variable(CompilerEngine* compiler, bool assign) -> void;
  auto friend Grammar::AndOp(CompilerEngine* compiler, bool assign) -> void;
  auto friend Grammar::OrOp(CompilerEngine* compiler, bool assign) -> void;
  auto friend Grammar::InvokeOp(CompilerEngine* compiler, bool assign) -> void;

 private:
  Token curr;
  Token prev;

  Compiler* compiler;
  CompilerEngine* parent;

  CompilerState state;
  u32 curr_func_idx = 0;
  Object::Function* curr_func = nullptr;

  u32 scope_depth = 0;
  u32 locals_count = 0;
  Local locals[Compiler::MAX_LOCALS_COUNT];
  Upvalue upvalues[Compiler::MAX_LOCALS_COUNT];
};

#undef VM_LEXEME_TYPE
#undef M_MAX_LOCALS_COUNT
