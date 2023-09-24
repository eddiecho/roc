#include "compiler.h"

#include <ctype.h>
#include <stdio.h>
#include <string.h>

#include "common.h"
#include "global_pool.h"
#include "object.h"
#include "string_pool.h"
#include "utils.h"
#include "value.h"

Token::Token() noexcept {
  this->type = Token::Lexeme::Eof;
  this->start = nullptr;
  this->len = 0;
  this->line = 0;
}

Token::Token(Token::Lexeme type, const char* start, u32 len, u32 line) noexcept
    : type{type}, start{start}, len{len}, line{line} {}

// @TODO(eddie) - I don't like this. Should be able to point to where the error
// starts
// @STDLIB
Token::Token(const char* error) noexcept {
  this->type = Token::Lexeme::Error;
  this->start = error;
  this->len = (u32)strlen(error);
  this->line = 0;
}

auto Token::IdentifiersEqual(Token other) const -> bool {
  if (this->len != other.len) return false;

  return memcmp(this->start, other.start, this->len) == 0;
}

auto static constexpr IsDigit(const char c) -> const bool {
  return c >= '0' && c <= '9';
}

auto static IsWhitespace(const char c) -> const bool {
  return c == ' ' || c == '\n' || c == '\t' || c == '\r';
}

// @STDLIB
auto static IsIdentifier(const char c) -> const bool {
  return !ispunct(c) && !isspace(c);
}

Scanner::Scanner() noexcept {
  this->start = nullptr;
  this->curr = nullptr;
  this->line = 0;
  this->row = 0;
}

auto Scanner::Init(const char* src) -> void {
  this->start = src;
  this->curr = src;
  this->line = 0;
  this->row = 0;
}

auto constexpr inline Scanner::IsEnd() const -> const bool {
  return *this->curr == '\0';
}

auto inline Scanner::Pop() -> char {
  this->curr++;
  this->row++;
  return this->curr[-1];
}

auto inline Scanner::MakeToken(Token::Lexeme type) const -> const Token {
  return Token{
      type,
      this->start,
      (u32)(this->curr - this->start),
      this->line,
  };
}

auto inline Scanner::Match(char expected) -> bool {
  if (this->IsEnd()) return false;
  if (*this->curr != expected) return false;
  this->curr++;
  return true;
}

auto constexpr inline Scanner::Peek() const -> const char {
  return *this->curr;
}

auto constexpr inline Scanner::PeekNext() const -> const char {
  return this->IsEnd() ? '\0' : this->curr[1];
}

auto Scanner::SkipWhitespace() -> void {
  while (1) {
    switch (this->Peek()) {
      case ' ':
      case '\r':
      case '\t': {
        this->Pop();
        break;
      }
      case '\n': {
        this->line++;
        this->row = 0;
        this->Pop();
        break;
      }
      default:
        return;
    }
  }
}

auto Scanner::StringToken() -> const Token {
  while (this->Peek() != '"' && !this->IsEnd()) {
    if (this->Peek() == '\n') this->line++;
    this->Pop();
  }

  if (this->IsEnd()) return Token("Unterminated string");

  this->Pop();
  return this->MakeToken(Token::Lexeme::String);
}

auto Scanner::NumberToken() -> const Token {
  while (IsDigit(this->Peek())) this->Pop();

  // you get one .
  if (this->Peek() == '.' && IsDigit(this->PeekNext())) {
    this->Pop();

    while (IsDigit(this->Peek())) this->Pop();
  }

  return this->MakeToken(Token::Lexeme::Number);
}

// @STDLIB
auto Scanner::CheckKeyword(u32 start, u32 length, const char* rest,
                          Token::Lexeme possible) const->const Token::Lexeme {
  if (this->curr - this->start == start + length &&
      memcmp(this->start + start, rest, length) == 0) {
    return possible;
  }

  return Token::Lexeme::Identifier;
}

auto Scanner::IdentifierType() -> const Token::Lexeme {
  while (IsIdentifier(this->Peek())) {
    this->Pop();
  }

  switch (*this->start) {
    case 'a':
      return this->CheckKeyword(1, 2, "nd", Token::Lexeme::And);
    case 'e':
      return this->CheckKeyword(1, 3, "lse", Token::Lexeme::Else);
    case 'f': {
      if (this->curr - this->start > 1) {
        switch (this->start[1]) {
          case 'a':
            return this->CheckKeyword(2, 3, "lse", Token::Lexeme::False);
          case 'o':
            return this->CheckKeyword(2, 1, "r", Token::Lexeme::For);
          case 'u':
            return this->CheckKeyword(2, 1, "n", Token::Lexeme::Function);
        }
      }
      break;
    }
    case 'i':
      if (this->curr - this->start > 1) {
        switch (this->start[1]) {
          case 'f':
            return this->CheckKeyword(2, 0, "", Token::Lexeme::If);
          case 'n':
            return this->CheckKeyword(2, 0, "", Token::Lexeme::In);
        }
      }
    case 'o':
      return this->CheckKeyword(1, 1, "r", Token::Lexeme::Or);
    case 'r':
      return this->CheckKeyword(1, 5, "eturn", Token::Lexeme::Return);
    case 's':
      return this->CheckKeyword(1, 5, "truct", Token::Lexeme::Struct);
    case 't':
      return this->CheckKeyword(1, 3, "rue", Token::Lexeme::True);
    case 'v':
      return this->CheckKeyword(1, 2, "ar", Token::Lexeme::Var);
    case 'w':
      return this->CheckKeyword(1, 4, "hile", Token::Lexeme::While);
  }

  return Token::Lexeme::Identifier;
}

auto Scanner::IdentifierToken() -> const Token {
  Token::Lexeme type = this->IdentifierType();
  return this->MakeToken(type);
}

auto Scanner::ScanToken() -> Token {
  this->SkipWhitespace();
  this->start = this->curr;

  if (this->IsEnd()) {
    return Token();
  }

  char c = this->Pop();
  switch (c) {
    case '(':
      return this->MakeToken(Token::Lexeme::LeftParens);
    case ')':
      return this->MakeToken(Token::Lexeme::RightParens);
    case '{':
      return this->MakeToken(Token::Lexeme::LeftBrace);
    case '}':
      return this->MakeToken(Token::Lexeme::RightBrace);
    case ';':
      return this->MakeToken(Token::Lexeme::Semicolon);
    case ':':
      return this->MakeToken(Token::Lexeme::Colon);
    case ',':
      return this->MakeToken(Token::Lexeme::Comma);
    case '.':
      return this->MakeToken(Token::Lexeme::Dot);
    case '-':
      return this->MakeToken(Token::Lexeme::Minus);
    case '+':
      return this->MakeToken(Token::Lexeme::Plus);
    case '*':
      return this->MakeToken(Token::Lexeme::Star);
    case '!':
      return this->MakeToken(this->Match('=') ? Token::Lexeme::BangEqual
                                              : Token::Lexeme::Bang);
    case '=':
      return this->MakeToken(this->Match('=') ? Token::Lexeme::EqualEqual
                                              : Token::Lexeme::Equal);
    case '<':
      return this->MakeToken(this->Match('=') ? Token::Lexeme::LessEqual
                                              : Token::Lexeme::Less);
    case '>':
      return this->MakeToken(this->Match('=') ? Token::Lexeme::GreaterEqual
                                              : Token::Lexeme::Greater);
    // @TODO(eddie) - Multiline comments
    case '/': {
      if (this->PeekNext() == '/') {
        while (this->Peek() != '\n' && !this->IsEnd()) this->Pop();
        return this->MakeToken(Token::Lexeme::Comment);
      } else {
        return this->MakeToken(Token::Lexeme::Slash);
      }
    }
    case '"':
      return this->StringToken();
    case '0':
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
    case '8':
    case '9':
      return this->NumberToken();
    // it's the wild west here baby
    default:
      return this->IdentifierToken();
  }
}

auto CompilerState::Merge(CompilerState other)->void {
  this->value |= other.value;
}

Compiler::Compiler() noexcept = default;

const ParseRuleMap Compiler::PARSE_RULES = {
    {Token::Lexeme::LeftParens,
     ParseRule(&Grammar::Parenthesis, &Grammar::InvokeOp, Precedence::Invoke)},
    {Token::Lexeme::RightParens, ParseRule(nullptr, nullptr, Precedence::None)},

    {Token::Lexeme::Minus,
     ParseRule(&Grammar::Unary, &Grammar::Binary, Precedence::Term)},
    {Token::Lexeme::Bang,
     ParseRule(&Grammar::Unary, nullptr, Precedence::None)},
    {Token::Lexeme::Plus,
     ParseRule(nullptr, &Grammar::Binary, Precedence::Term)},
    {Token::Lexeme::Slash,
     ParseRule(nullptr, &Grammar::Binary, Precedence::Factor)},
    {Token::Lexeme::Star,
     ParseRule(nullptr, &Grammar::Binary, Precedence::Factor)},

    {Token::Lexeme::Greater,
     ParseRule(nullptr, &Grammar::Binary, Precedence::Comparison)},
    {Token::Lexeme::GreaterEqual,
     ParseRule(nullptr, &Grammar::Binary, Precedence::Comparison)},
    {Token::Lexeme::Less,
     ParseRule(nullptr, &Grammar::Binary, Precedence::Comparison)},
    {Token::Lexeme::LessEqual,
     ParseRule(nullptr, &Grammar::Binary, Precedence::Comparison)},
    {Token::Lexeme::BangEqual,
     ParseRule(nullptr, &Grammar::Binary, Precedence::Equality)},
    {Token::Lexeme::EqualEqual,
     ParseRule(nullptr, &Grammar::Binary, Precedence::Equality)},

    {Token::Lexeme::Number,
     ParseRule(&Grammar::Number, nullptr, Precedence::None)},

    {Token::Lexeme::False,
     ParseRule(&Grammar::Literal, nullptr, Precedence::None)},
    {Token::Lexeme::True,
     ParseRule(&Grammar::Literal, nullptr, Precedence::None)},
    {Token::Lexeme::String,
     ParseRule(&Grammar::String, nullptr, Precedence::None)},

    {Token::Lexeme::And, ParseRule(nullptr, &Grammar::AndOp, Precedence::And)},
    {Token::Lexeme::Or, ParseRule(nullptr, &Grammar::OrOp, Precedence::Or)},
    {Token::Lexeme::LeftBrace, ParseRule(nullptr, nullptr, Precedence::None)},
    {Token::Lexeme::RightBrace, ParseRule(nullptr, nullptr, Precedence::None)},
    {Token::Lexeme::Eof, ParseRule(nullptr, nullptr, Precedence::None)},
    {Token::Lexeme::Identifier,
     ParseRule(&Grammar::Variable, nullptr, Precedence::None)},

    // @TODO(eddie)
    {Token::Lexeme::Error, ParseRule(nullptr, nullptr, Precedence::None)},
    {Token::Lexeme::Comment, ParseRule(nullptr, nullptr, Precedence::None)},
    {Token::Lexeme::Comma, ParseRule(nullptr, nullptr, Precedence::None)},
    {Token::Lexeme::Dot, ParseRule(nullptr, nullptr, Precedence::None)},
    {Token::Lexeme::Semicolon, ParseRule(nullptr, nullptr, Precedence::None)},
    {Token::Lexeme::Colon, ParseRule(nullptr, nullptr, Precedence::None)},
    {Token::Lexeme::Equal, ParseRule(nullptr, nullptr, Precedence::None)},
    {Token::Lexeme::Else, ParseRule(nullptr, nullptr, Precedence::None)},
    {Token::Lexeme::For, ParseRule(nullptr, nullptr, Precedence::None)},
    {Token::Lexeme::Function, ParseRule(nullptr, nullptr, Precedence::None)},
    {Token::Lexeme::If, ParseRule(nullptr, nullptr, Precedence::None)},
    {Token::Lexeme::Return, ParseRule(nullptr, nullptr, Precedence::None)},
    {Token::Lexeme::Struct, ParseRule(nullptr, nullptr, Precedence::None)},
    {Token::Lexeme::Var, ParseRule(nullptr, nullptr, Precedence::None)},
    {Token::Lexeme::While, ParseRule(nullptr, nullptr, Precedence::None)},

};

auto Compiler::Init(const char* src, StringPool* string_pool,
                   GlobalPool* global_pool)
    ->void {
  this->string_pool = string_pool;
  this->global_pool = global_pool;
  this->scanner.Init(src);
}

auto Compiler::Compile()->Result<Object*, CompileError> {
  CompilerEngine engine = {};
  engine.Init(this);

  return engine.Compile();
}

auto inline CompilerEngine::Init(Compiler* compiler)->void {
  this->Init(compiler, Compiler::GLOBAL_FUNCTION_NAME_LEN,
             Compiler::GLOBAL_FUNCTION_NAME);
}

auto CompilerEngine::Init(Compiler* compiler, u32 name_length, const char* name)
    ->void {
  this->compiler = compiler;
  this->parent = nullptr;

  auto func_idx = compiler->global_pool->Alloc(name_length, name);
  auto curr_func =
      static_cast<Object::Function*>(compiler->global_pool->Nth(func_idx));

  auto name_idx = this->AddString(name_length, name);
  auto interned_name = compiler->string_pool->Nth(name_idx);

  Chunk* chunk = compiler->chunk_manager.Alloc();
  curr_func->Init(chunk, name_length, interned_name->name);

  this->curr_func = curr_func;
  this->curr_func_idx = func_idx;

  // steal the first local slot for function
  // makes the rest cleaner i guess
  // especially when you have to call functions or something
  Local* top_level_declaration = &this->locals[this->locals_count++];
  top_level_declaration->depth = 0;
  top_level_declaration->id.start = "";
  top_level_declaration->id.len = 0;
  top_level_declaration->captured = false;
}

auto CompilerEngine::Compile() -> CompileResult {
  this->Advance();

  while (this->curr.type != Token::Lexeme::Eof) {
    this->Declaration();
  }

  this->EndCompilation();

  if (this->state.error) return CompileError::Syntax;

  return this->curr_func;
}

auto inline CompilerEngine::CurrentChunk() -> Chunk* {
  return this->curr_func->as.function.chunk;
}

auto CompilerEngine::Declaration() -> void {
  switch (this->curr.type) {
    default: {
      this->Statement();
      break;
    }
    case Token::Lexeme::Var: {
      this->Advance();
      this->VariableDeclaration();
      break;
    }
    case Token::Lexeme::Function: {
      this->Advance();
      this->FunctionDeclaration();
    }
  }

  if (this->state.panic) this->SyncOnError();
}

auto CompilerEngine::Statement() -> void {
  // @NOTE(eddie)
  // the only kinds of statements in this lang are
  // return/yield
  // var/func/struct declarations

  if (this->curr.type == Token::Lexeme::Return) {
    this->Advance();

    if (this->scope_depth == 0) {
      this->ErrorAtCurr("Can not have top level return statement");
    }

    if (this->curr.type == Token::Lexeme::Semicolon) {
      this->Advance();
      this->Emit(OpCode::ReturnVoid);
    } else {
      this->Expression();
      this->Emit(OpCode::Return);
    }

  } else {
    // technically, expressions without a corresponding assignment or the above
    // are called "expression statements" basically just calling a function and
    // discarding the return value
    this->Expression();
  }
}

auto CompilerEngine::Expression(const bool nested)->void {
  switch (this->curr.type) {
    case Token::Lexeme::If: {
      this->Advance();
      // this is the condition
      // uhm, does this mean nested if expressions work?
      this->Expression(true);

      const u32 if_jump_idx = this->Jump(OpCode::JumpFalse);
      this->Emit(OpCode::Pop);

      // this is the block after
      this->Statement();

      const u32 else_jump_idx = this->Jump(OpCode::Jump);
      this->PatchJump(if_jump_idx);
      this->Emit(OpCode::Pop);

      if (this->curr.type == Token::Lexeme::Else) {
        this->Statement();
      }

      this->PatchJump(else_jump_idx);
      break;
    }
    case Token::Lexeme::While: {
      this->Advance();
      const u64 loop_start = this->CurrentChunk()->Count();

      this->Expression(true);

      const u32 exit_jump = this->Jump(OpCode::JumpFalse);
      this->Emit(OpCode::Pop);

      this->Statement();

      this->Loop(loop_start);

      this->PatchJump(exit_jump);
      this->Emit(OpCode::Pop);
      break;
    }
    case Token::Lexeme::For: {
      this->Advance();
      // @TODO(eddie) - we need range generators and iterators
      // @TODO(eddie) - continue and break statements
      this->BeginScope();

      this->VariableDeclaration();

      // the block
      this->Expression(true);

      this->EndScope();
      break;
    }
    case Token::Lexeme::LeftBrace: {
      this->Advance();
      this->BeginScope();
      this->CodeBlock();
      this->EndScope();
      break;
    }
    default: {
      this->GetPrecedence(Precedence::Assignment);

      if (!nested)
        this->Consume(Token::Lexeme::Semicolon,
                      "Expected semicolon ';' after non block expression.");

      break;
    }
  }
}

auto CompilerEngine::Jump(OpCode kind)->u32 {
  this->Emit(kind);
  int placeholder = 0xFFFFFFFF;
  const auto place = IntToBytes(&placeholder);
  this->Emit(place, 4);

  return this->CurrentChunk()->Count() - 4;
}

auto CompilerEngine::PatchJump(u64 offset) -> void {
  const u32 jump = this->CurrentChunk()->Count() - offset - 4;

  // I HAVE NO FUCKING CLUE WHICH ENDIAN IS WHICH
  // so! we just ignore it by doing disgusting bit reinterpreting
  const auto code = this->CurrentChunk()->bytecode.data + offset;
  auto as_int = reinterpret_cast<u32*>(code);
  *as_int = jump;
}

auto CompilerEngine::Loop(u64 loop_start) -> void {
  this->Emit(OpCode::Loop);
  u32 offset = this->CurrentChunk()->Count() - loop_start + 2;

  this->Emit(IntToBytes(&offset), 4);
}

auto CompilerEngine::SyncOnError() -> void {
  this->state.panic = 0;

  while (this->curr.type != Token::Lexeme::Eof) {
    if (this->prev.type == Token::Lexeme::Semicolon) return;

    switch (this->curr.type) {
      default:
        continue;
      case Token::Lexeme::Struct:
      case Token::Lexeme::Function:
      case Token::Lexeme::Var:
      case Token::Lexeme::For:
      case Token::Lexeme::If:
      case Token::Lexeme::While:
      case Token::Lexeme::Return:
        return;
    }

    this->Advance();
  }
}

auto CompilerEngine::BeginScope()->void { this->scope_depth++; }

auto CompilerEngine::EndScope()->void {
  this->scope_depth--;

  // @TODO(eddie) - PopN opcode, because this sucks
  while (this->locals_count > 0 &&
         this->locals[this->locals_count - 1].depth > this->scope_depth) {
    const auto local = this->locals[this->locals_count - 1];
    const auto op = local.captured ? OpCode::CloseUpvalue : OpCode::Pop;

    this->Emit(op);
    this->locals_count--;
  }
}

auto CompilerEngine::CodeBlock()->void {
  while (this->curr.type != Token::Lexeme::RightBrace &&
         this->curr.type != Token::Lexeme::Eof) {
    this->Declaration();
  }

  this->Consume(Token::Lexeme::RightBrace, "Expected '}' to end block");
}

auto CompilerEngine::Advance()->void {
  u32 line = 0xFFFFFFFF;
  Token token;
  this->prev = this->curr;

  while (1) {
    token = this->compiler->scanner.ScanToken();
    this->curr = token;

#ifdef DEBUG_TRACE_EXECUTION
    if (token.line != line) {
      printf("%4d ", token.line);
    } else {
      printf("   | ");
    }
    printf("%s '%.*s'\n", token.Print(), token.len, token.start);
#endif

    if (this->curr.type != Token::Lexeme::Error) return;

    this->ErrorAtCurr(this->curr.start);
  }
}

// @TODO(eddie) - just get rid of this
auto inline CompilerEngine::MatchAndAdvance(Token::Lexeme type) -> bool {
  if (this->curr.type == type) {
    this->Advance();
    return true;
  }

  return false;
}

auto inline CompilerEngine::ErrorAtToken(const char* message, Token id) -> void {
  if (this->state.panic) return;

  fprintf(stderr, "[line %d] Error", id.line);
  fprintf(stderr, " at '%.*s'", id.len, id.start);
  fprintf(stderr, ": %s\n", message);

  this->state.panic = 1;
  this->state.error = 1;
}

auto inline CompilerEngine::ErrorAtCurr(const char* message) -> void {
  this->ErrorAtToken(message, this->curr);
}

auto CompilerEngine::VariableDeclaration() -> void {
  const bool in_for_loop = this->curr.type == Token::Lexeme::For;

  this->Consume(Token::Lexeme::Identifier, "Expected variable name");
  if (this->scope_depth == 0) {
    this->AddGlobal(this->prev);
  } else {
    this->AddLocal(this->prev);
  }

  if (in_for_loop && this->curr.type == Token::Lexeme::In) {
    this->Advance();
    this->Expression();
  } else if (this->curr.type == Token::Lexeme::Equal) {
    this->Advance();
    this->Expression();
  }
}

auto CompilerEngine::FunctionDeclaration() -> void {
  const auto declaration_line = this->curr.line;

  this->Consume(Token::Lexeme::Identifier, "Expected function name");

  CompilerEngine new_engine = {};
  new_engine.Init(this->compiler, this->prev.len, this->prev.start);
  new_engine.curr = this->curr;
  new_engine.prev = this->prev;
  new_engine.parent = this;

  const u32 func_start = this->prev.line;

  new_engine.FunctionBody();

  this->curr = new_engine.curr;
  this->prev = new_engine.prev;
  // is this necessary?
  // new_engine.EndCompilation();
  const auto func = new_engine.curr_func;

  /*
  u32 func_idx = this->CurrentChunk()->AddLocal(Value(func), func_start);
  this->Emit(OpCode::GetLocal);
  this->Emit(IntToBytes(&func_idx), 4);
  */

  if (new_engine.state.has_captures) {
    this->Emit(OpCode::Closure);
    this->Emit(IntToBytes(&new_engine.curr_func_idx), 4);
    this->Emit(func->as.function.upvalue_count);

    for (int i = 0; i < func->as.function.upvalue_count; i++) {
      this->Emit(this->upvalues[i].local ? 1 : 0);
      this->Emit(this->upvalues[i].index);
    }
  }
}

auto CompilerEngine::FunctionBody()->void {
  this->BeginScope();
  this->Consume(Token::Lexeme::LeftParens,
                "Functions require function parameters starting with '(");

  if (this->curr.type != Token::Lexeme::RightParens) {
    do {
      this->curr_func->as.function.arity++;
      this->VariableDeclaration();

    } while (this->MatchAndAdvance(Token::Lexeme::Comma));
  }

  this->Consume(Token::Lexeme::RightParens,
                "No closing parentheses for function parameters");
  this->Consume(Token::Lexeme::LeftBrace,
                "Expect code block following function declaration");

  this->CodeBlock();
  this->EndScope();
}

auto inline CompilerEngine::AddString(u32 length, const char* start)->u32 {
  return this->compiler->string_pool->Alloc(length, start);
}

auto inline CompilerEngine::AddGlobal(Token id)->u64 {
  return this->compiler->global_pool->Alloc(id.len, id.start);
}

auto CompilerEngine::AddLocal(Token id)->void {
  // @TODO(eddie) - this sucks
  if (this->locals_count == Compiler::MAX_LOCALS_COUNT) {
    this->ErrorAtCurr("Too many locals in current scope");
  }

  Local* local = &this->locals[this->locals_count++];
  local->id = id;
  local->depth = this->scope_depth;
  local->captured = false;
}

auto CompilerEngine::AddUpvalue(u8 index, bool local) -> u32 {
  const u32 count = this->curr_func->as.function.upvalue_count;
  this->upvalues[count].local = local;
  this->upvalues[count].index = index;
  return this->curr_func->as.function.upvalue_count++;
}

auto inline CompilerEngine::FindLocal(Token id) -> Option<u64> {
  return this->FindLocal(id, this->locals);
}

auto CompilerEngine::FindLocal(Token id, Local* scope) -> Option<u64> {
  for (int i = this->locals_count - 1; i >= 0; i--) {
    const Local* local = &this->locals[i];
    if (local->id.IdentifiersEqual(id)) {
      return i;
    }
  }

  return OptionType::None;
}

auto CompilerEngine::FindUpvalue(Token id) -> Option<u64> {
  if (this->parent == nullptr) return OptionType::None;

  auto idx = this->FindLocal(id, this->parent->locals);
  if (!idx.IsNone()) {
    const auto got = idx.Get();
    this->parent->locals[got].captured = true;
    return this->AddUpvalue(got, true);
  }

  idx = this->parent->FindUpvalue(id);
  if (!idx.IsNone()) {
    return this->AddUpvalue(idx.Get(), false);
  }

  return OptionType::None;
}

auto inline CompilerEngine::FindGlobal(Token id) -> Option<u64> {
  return this->compiler->global_pool->Find(id.len, id.start);
}

/*
 * Variables work like this:
 * GlobalPool is backed by a hash table, keyed by variable name
 * the value is an index into a buffer of Objects. The hash table
 * does not exist at runtime.
 *
 * Locals are stored directly into a Chunk's locals array
 * A Local's name is erased at runtime @TODO(eddie) - this bad from a debugging
 * POV Lookups are handled in CompilerEngine, we store an array of names here,
 * and lookups return an index into the locals array
 */
auto CompilerEngine::LoadVariable(bool assignment)->void {
  OpCode get = {};
  OpCode set = {};

  // This implicitly defines a hierarchy for variable resolution
  // Locals have priority over globals,
  // globals have priorty over upvalues;
  // @TODO(eddie) - i wonder if we should adjust this?
  // also the conditionals are kinda cursed
  auto idx = this->FindLocal(this->prev);
  if (!idx.IsNone()) {
    get = OpCode::GetLocal;
    set = OpCode::SetLocal;
  } else if (idx = this->FindGlobal(this->prev); !idx.IsNone()) {

    /*
    auto obj = this->compiler->global_pool->Nth(idx.Get());
    if (obj->type == ObjectType::Function) {
      return;
    }
    */

    get = OpCode::GetGlobal;
    set = OpCode::SetGlobal;
  } else if (idx = this->FindUpvalue(this->prev); !idx.IsNone()) {
    get = OpCode::GetUpvalue;
    set = OpCode::SetUpvalue;
    this->state.has_captures = 1;
  } else {
    this->ErrorAtToken("Undefined variable", this->prev);
    return;
  }

  if (this->curr.type == Token::Lexeme::Equal && assignment) {
    this->Advance();
    this->Expression();

    this->Emit(set);
  } else {
    this->Emit(get);
  }

  auto unwrapped = idx.Get();
  // @FIXME(eddie) - this is 8 bytes
  this->Emit(IntToBytes(&unwrapped), 4);
}

auto inline CompilerEngine::Emit(u8 byte) -> void {
  this->CurrentChunk()->AddInstruction(byte, this->prev.line);
}

auto inline CompilerEngine::Emit(u8* bytes, u32 count) -> void {
  this->CurrentChunk()->AddInstruction(bytes, count, this->prev.line);
}

auto inline CompilerEngine::Emit(OpCode op) -> void {
  const u8 byte = static_cast<u8>(op);
  this->CurrentChunk()->AddInstruction(byte, this->prev.line);
}

auto CompilerEngine::Consume(Token::Lexeme type, const char* message) -> void {
  if (this->curr.type == type) {
    this->Advance();

    return;
  }

  this->ErrorAtCurr(message);
}

auto CompilerEngine::EndCompilation() -> void { this->Emit(OpCode::ReturnVoid); }

auto inline CompilerEngine::GetParseRule(Token::Lexeme token) -> const ParseRule* {
  return &this->compiler->PARSE_RULES.at(token);
}

auto CompilerEngine::GetPrecedence(Precedence precedence) -> void {
  this->Advance();
  const ParseRule* rule = this->GetParseRule(this->prev.type);

  if (rule->prefix == nullptr) {
    this->ErrorAtCurr("Expected expression");
    return;
  }

  const u32 assignment_prec = static_cast<u32>(Precedence::Assignment);
  const u32 curr_prec = static_cast<u32>(precedence);
  const bool assign = curr_prec <= assignment_prec;

  rule->prefix(this, assign);

  while (precedence <= this->GetParseRule(this->curr.type)->precedence) {
    this->Advance();
    const ParseRule* prev_rule = this->GetParseRule(this->prev.type);
    prev_rule->infix(this, assign);
  }

  if (assign && this->curr.type == Token::Lexeme::Equal) {
    this->Advance();
    this->ErrorAtCurr("Invalid assignment target.");
  }
}

// @STDLIB
auto static Grammar::Number(CompilerEngine* compiler, bool assign) -> void {
  const f64 value = strtod(compiler->prev.start, nullptr);
  compiler->CurrentChunk()->AddLocal(Value(value), compiler->prev.line);
}

auto static Grammar::Parenthesis(CompilerEngine* compiler, bool assign) -> void {
  compiler->Expression(true);
  compiler->Consume(Token::Lexeme::RightParens, "Expect ')' after expression");
}

auto static Grammar::Unary(CompilerEngine* compiler, bool assign) -> void {
  const Token::Lexeme op = compiler->prev.type;
  compiler->GetPrecedence(Precedence::Unary);

  switch (op) {
    default:
      return;
    case Token::Lexeme::Minus: {
      compiler->Emit(OpCode::Negate);
      break;
    }
    case Token::Lexeme::Bang: {
      compiler->Emit(OpCode::Not);
      break;
    }
  }
}

auto static Grammar::Binary(CompilerEngine* compiler, bool assign) -> void {
  const Token::Lexeme op = compiler->prev.type;

  const int higher = static_cast<int>(Precedence::Term) + 1;
  const auto next_higher = static_cast<Precedence>(higher);
  compiler->GetPrecedence(next_higher);

  switch (op) {
    default:
      return;
    case Token::Lexeme::Plus: {
      compiler->Emit(OpCode::Add);
      break;
    }
    case Token::Lexeme::Minus: {
      compiler->Emit(OpCode::Subtract);
      break;
    }
    case Token::Lexeme::Star: {
      compiler->Emit(OpCode::Multiply);
      break;
    }
    case Token::Lexeme::Slash: {
      compiler->Emit(OpCode::Divide);
      break;
    }
    case Token::Lexeme::BangEqual: {
      compiler->Emit(OpCode::Equality);
      compiler->Emit(OpCode::Not);
      break;
    }
    case Token::Lexeme::EqualEqual: {
      compiler->Emit(OpCode::Equality);
      break;
    }
    case Token::Lexeme::Greater: {
      compiler->Emit(OpCode::Greater);
      break;
    }
    case Token::Lexeme::GreaterEqual: {
      compiler->Emit(OpCode::Greater);
      compiler->Emit(OpCode::Not);
      break;
    }
    case Token::Lexeme::Less: {
      compiler->Emit(OpCode::Less);
      break;
    }
    case Token::Lexeme::LessEqual: {
      compiler->Emit(OpCode::Less);
      compiler->Emit(OpCode::Not);
      break;
    }
  }
}

auto static Grammar::Literal(CompilerEngine* compiler, bool assign) -> void {
  switch (compiler->prev.type) {
    default:
      return;  // unreachable
    case Token::Lexeme::False: {
      compiler->Emit(OpCode::False);
      break;
    }
    case Token::Lexeme::True: {
      compiler->Emit(OpCode::True);
      break;
    }
  }
}

auto static Grammar::String(CompilerEngine* compiler, bool assign) -> void {
  // skip the closing quote
  const u32 length = compiler->prev.len - 2;
  u32 index = compiler->AddString(length, compiler->prev.start + 1);

  compiler->Emit(OpCode::String);
  compiler->Emit(IntToBytes(&index), 4);
}

auto static Grammar::Variable(CompilerEngine* compiler, bool assign) -> void {
  compiler->LoadVariable(assign);
}

auto static Grammar::AndOp(CompilerEngine* compiler, bool assign) -> void {
  u32 short_circuit = compiler->Jump(OpCode::JumpFalse);

  compiler->Emit(OpCode::Pop);
  compiler->GetPrecedence(Precedence::And);
  compiler->PatchJump(short_circuit);
}

auto static Grammar::OrOp(CompilerEngine* compiler, bool assign) -> void {
  const u32 short_circuit = compiler->Jump(OpCode::JumpTrue);

  compiler->Emit(OpCode::Pop);
  compiler->GetPrecedence(Precedence::Or);
  compiler->PatchJump(short_circuit);
}

auto static Grammar::InvokeOp(CompilerEngine* compiler, bool assign) -> void {
  u32 arg_count = 0;
  if (compiler->curr.type != Token::Lexeme::RightParens) {
    do {
      compiler->Expression(true);
      arg_count++;
    } while (compiler->MatchAndAdvance(Token::Lexeme::Comma));
  }

  compiler->Consume(Token::Lexeme::RightParens,
                    "Expected closing parenthesis ')' after arguments");

  compiler->Emit(OpCode::Invoke);
  compiler->Emit(IntToBytes(&arg_count), 4);
}
