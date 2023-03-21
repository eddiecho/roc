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

fnc Token::IdentifiersEqual(Token other) const -> bool {
  if (this->len != other.len) return false;

  return memcmp(this->start, other.start, this->len) == 0;
}

fnc static constexpr IsDigit(const char c) -> const bool {
  return c >= '0' && c <= '9';
}

fnc static IsWhitespace(const char c) -> const bool {
  return c == ' ' || c == '\n' || c == '\t' || c == '\r';
}

// @STDLIB
fnc static IsIdentifier(const char c) -> const bool {
  return !ispunct(c) && !isspace(c);
}

Scanner::Scanner() noexcept {
  this->start = nullptr;
  this->curr = nullptr;
  this->line = 0;
  this->row = 0;
}

fnc Scanner::Init(const char* src) -> void {
  this->start = src;
  this->curr = src;
  this->line = 0;
  this->row = 0;
}

fnc constexpr inline Scanner::IsEnd() const -> const bool {
  return *this->curr == '\0';
}

fnc inline Scanner::Pop() -> char {
  this->curr++;
  this->row++;
  return this->curr[-1];
}

fnc inline Scanner::MakeToken(Token::Lexeme type) const -> const Token {
  return Token{
      type,
      this->start,
      (u32)(this->curr - this->start),
      this->line,
  };
}

fnc inline Scanner::Match(char expected) -> bool {
  if (this->IsEnd()) return false;
  if (*this->curr != expected) return false;
  this->curr++;
  return true;
}

fnc constexpr inline Scanner::Peek() const -> const char {
  return *this->curr;
}

fnc constexpr inline Scanner::PeekNext() const -> const char {
  return this->IsEnd() ? '\0' : this->curr[1];
}

fnc Scanner::SkipWhitespace() -> void {
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

fnc Scanner::StringToken() -> const Token {
  while (this->Peek() != '"' && !this->IsEnd()) {
    if (this->Peek() == '\n') this->line++;
    this->Pop();
  }

  if (this->IsEnd()) return Token("Unterminated string");

  this->Pop();
  return this->MakeToken(Token::Lexeme::String);
}

fnc Scanner::NumberToken() -> const Token {
  while (IsDigit(this->Peek())) this->Pop();

  // you get one .
  if (this->Peek() == '.' && IsDigit(this->PeekNext())) {
    this->Pop();

    while (IsDigit(this->Peek())) this->Pop();
  }

  return this->MakeToken(Token::Lexeme::Number);
}

// @STDLIB
fnc Scanner::CheckKeyword(u32 start, u32 length, const char* rest,
                           Token::Lexeme possible) const
    -> const Token::Lexeme {
  if (this->curr - this->start == start + length &&
      memcmp(this->start + start, rest, length) == 0) {
    return possible;
  }

  return Token::Lexeme::Identifier;
}

fnc Scanner::IdentifierType() -> const Token::Lexeme {
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

fnc Scanner::IdentifierToken() -> const Token {
  Token::Lexeme type = this->IdentifierType();
  return this->MakeToken(type);
}

fnc Scanner::ScanToken() -> Token {
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

Compiler::Compiler() noexcept {}

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
    {Token::Lexeme::Identifier, ParseRule(&Grammar::Variable, nullptr, Precedence::None)},

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

fnc Compiler::Init(const char* src,
                   StringPool* string_pool,
                   GlobalPool* global_pool)
    -> void {
  this->string_pool = string_pool;
  this->global_pool = global_pool;
  this->scanner.Init(src);

  auto top_level_func_idx = this->global_pool->Alloc(
      Compiler::GLOBAL_FUNCTION_NAME_LEN, Compiler::GLOBAL_FUNCTION_NAME);
  this->curr_func = static_cast<Object::Function*>(this->global_pool->Nth(top_level_func_idx));
  this->curr_func->as.function.arity = 0;
  this->curr_func->type = ObjectType::Function;
  Chunk* chunk = this->chunk_manager.Alloc();
  this->curr_func->as.function.chunk = *chunk;

  // steal the first local slot for the top level function
  // makes the rest cleaner i guess
  // especially when you have to call functions or something
  Local* top_level_declaration = &this->locals.locals[this->locals.locals_count++];
  top_level_declaration->depth = 0;
  top_level_declaration->id.start = "";
  top_level_declaration->id.len = 0;
}

fnc inline Compiler::CurrentChunk() -> Chunk* {
  return &this->curr_func->as.function.chunk;
}

fnc Compiler::Compile() -> Result<Object*, CompileError> {
  this->Advance();

  while (!this->Match(Token::Lexeme::Eof)) {
    this->Declaration();
  }

  this->EndCompilation();

  if (this->state.error) return CompileError::Syntax;

  return this->curr_func;
}

fnc Compiler::Declaration() -> void {
  if (this->Match(Token::Lexeme::Var)) {
    this->VariableDeclaration();
  } else if (this->Match(Token::Lexeme::Function)) {
    this->FunctionDeclaration();
  } else {
    this->Statement();
  }

  if (this->state.panic) this->SyncOnError();
}

fnc Compiler::Statement() -> void {
  // @NOTE(eddie)
  // the only kinds of statements in this lang are
  // return/yield
  // var/func/struct declarations

  if (this->curr.type == Token::Lexeme::Return) {
    if (this->scope_depth == 0) {
      this->ErrorAtCurr("Can not have top level return statement");
    }

    if (this->Match(Token::Lexeme::Semicolon)) {
      this->Emit(OpCode::Return);
    } else {
      this->Expression();
      this->Emit(OpCode::Return);
    }

  } else {
    // technically, expressions without a corresponding assignment or the above are called
    // "expression statements"
    // basically just calling a function and discarding the return value
    this->Expression();
  }
}

fnc Compiler::Expression(bool nested) -> void {
  if (this->Match(Token::Lexeme::If)) {
    // this is the condition
    // @FIXME(eddie) - uhm, does this mean nested if expressions work?
    this->Expression();

    u32 if_jump_idx = this->Jump(OpCode::JumpFalse);
    this->Emit(OpCode::Pop);

    // this is the block after
    this->Statement();

    u32 else_jump_idx = this->Jump(OpCode::Jump);
    this->PatchJump(if_jump_idx);
    this->Emit(OpCode::Pop);

    if (this->Match(Token::Lexeme::Else)) {
      this->Statement();
    }
    this->PatchJump(else_jump_idx);
  } else if (this->Match(Token::Lexeme::While)) {
    u64 loop_start = this->CurrentChunk()->Count();

    this->Expression();

    u32 exit_jump = this->Jump(OpCode::JumpFalse);
    this->Emit(OpCode::Pop);

    this->Statement();

    this->Loop(loop_start);

    this->PatchJump(exit_jump);
    this->Emit(OpCode::Pop);

  // @TODO(eddie) - we need range generators and iterators
  // @TODO(eddie) - continue and break statements
  } else if (this->Match(Token::Lexeme::For)) {
    this->BeginScope();

    this->VariableDeclaration();

    // the block
    this->Expression();

    this->EndScope();
  } else if (this->Match(Token::Lexeme::LeftBrace)) {
    this->BeginScope();
    this->CodeBlock();
    this->EndScope();
  } else {
    this->GetPrecedence(Precedence::Assignment);

    if (!nested)
      this->Consume(Token::Lexeme::Semicolon, "Expected semicolon ';' after non block expression.");
  }
}

fnc Compiler::Jump(OpCode kind) -> u32 {
  this->Emit(kind);
  this->Emit(IntToBytes(0xFFFFFFFF), 4);

  return this->CurrentChunk()->Count() - 4;
}

fnc Compiler::PatchJump(u64 offset) -> void {
  u32 jump = this->CurrentChunk()->Count() - offset - 2;

  this->CurrentChunk()->bytecode[offset] = (jump >> 24) & 0xFF;
  this->CurrentChunk()->bytecode[offset + 1] = (jump >> 16) & 0xFF;
  this->CurrentChunk()->bytecode[offset + 2] = (jump >> 8) & 0xFF;
  this->CurrentChunk()->bytecode[offset + 3] = jump & 0xFF;
}

fnc Compiler::Loop(u64 loop_start) -> void {
  this->Emit(OpCode::Loop);
  u32 offset = this->CurrentChunk()->Count() - loop_start + 2;

  this->Emit(IntToBytes(&offset), 4);
}

fnc Compiler::SyncOnError() -> void {
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

fnc Compiler::BeginScope() -> void {
  this->scope_depth++;
}

fnc Compiler::EndScope() -> void {
  this->scope_depth--;

  // @TODO(eddie) - PopN opcode, because this sucks
  while (this->locals.locals_count > 0
    && this->locals.locals[this->locals.locals_count - 1].depth > this->scope_depth) {
    this->Emit(OpCode::Pop);
    this->locals.locals_count--;
  }
}

fnc Compiler::CodeBlock() -> void {
  while (this->curr.type != Token::Lexeme::RightBrace
    && this->curr.type != Token::Lexeme::Eof) {
    this->Declaration();
  }

  this->Consume(Token::Lexeme::RightBrace, "Expected '}' to end block");
}

fnc Compiler::Advance() -> void {
  u32 line = 0xFFFFFFFF;
  Token token;
  this->prev = this->curr;

  while (1) {
    token = this->scanner.ScanToken();
    this->curr = token;

#ifdef DEBUG_TRACE_EXECUTION
    // output debug token info
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
fnc inline Compiler::Match(Token::Lexeme type) -> bool {
  if (this->curr.type == type) {
    this->Advance();
    return true;
  }

  return false;
}

fnc Compiler::ErrorAtCurr(const char* message) -> void {
  if (this->state.panic) return;

  fprintf(stderr, "[line %d] Error", this->curr.line);
  fprintf(stderr, " at '%.*s'", this->curr.len, this->curr.start);
  fprintf(stderr, ": %s\n", message);

  this->state.panic = 1;
  this->state.error = 1;
}

fnc Compiler::VariableDeclaration() -> void {
  bool in_for_loop = this->curr.type == Token::Lexeme::For;

  this->Consume(Token::Lexeme::Identifier, "Expected variable name");
  if (this->scope_depth == 0) {
    this->AddGlobal(this->prev);
  } else {
    this->AddLocal(this->prev);
  }

  if (in_for_loop && this->Match(Token::Lexeme::In)) {
    this->Expression();
  } else if (this->Match(Token::Lexeme::Equal)) {
    this->Expression();
  }
}

fnc Compiler::FunctionDeclaration() -> void {
  auto declaration_line = this->curr.line;

  this->Consume(Token::Lexeme::Identifier, "Expected function name");

  auto new_func_idx = this->AddGlobal(this->prev);
  Object::Function* new_func = static_cast<Object::Function*>(
      this->global_pool->Nth(new_func_idx));

  auto chunk = this->chunk_manager.Alloc();
  new_func->type = ObjectType::Function;
  new_func->as.function.Init(*chunk);
  new_func->next = (Object*)this->curr_func;
  new_func->name_len = this->prev.len;

  u64 name_idx = this->string_pool->Alloc(this->prev.len, this->prev.start);
  new_func->name = this->string_pool->Nth(name_idx)->name;

  ScopedLocals new_locals = {};
  new_locals.prev = &this->locals;
  this->locals = new_locals;
  this->curr_func = new_func;

  // see Compiler::Init() for why we need this
  Local* base_local = &this->locals.locals[this->locals.locals_count++];
  base_local->depth = 0;
  base_local->id.start = "";
  base_local->id.len = 0;

  u32 func_start = this->prev.line;

  this->BeginScope();

  this->Consume(Token::Lexeme::LeftParens, "Functions require function parameters starting with '(");

  if (this->curr.type != Token::Lexeme::RightParens) {
    do {
      this->curr_func->as.function.arity++;
      this->VariableDeclaration();

    } while(this->Match(Token::Lexeme::Comma));
  }

  this->Consume(Token::Lexeme::RightParens, "No closing parentheses for function parameters");
  this->Consume(Token::Lexeme::LeftBrace, "Expect code block following function declaration");

  this->CodeBlock();

  this->EndScope();

  // reset the current function to whatever it was before
  this->locals = *this->locals.prev;
  this->curr_func = static_cast<Object::Function*>(this->curr_func->next);

  // we need this only for closures that actually require upvalues
  /*
  u32 func_idx = this->CurrentChunk()->AddConstant(Value(new_func), declaration_line);
  this->Emit(OpCode::Closure);
  this->Emit(IntToBytes(&func_idx), 4);
    */
}

fnc Compiler::AddGlobal(Token id) -> u64 {
  return this->global_pool->Alloc(id.len, id.start);
}

fnc Compiler::AddLocal(Token id) -> void {
  // @TODO(eddie) - this sucks
  if (this->locals.locals_count == Compiler::MAX_LOCALS_COUNT) {
    this->ErrorAtCurr("Too many locals in current scope");
  }

  Local* local = &this->locals.locals[this->locals.locals_count++];
  local->id = id;
  local->depth = this->scope_depth;
}

fnc Compiler::FindLocal(Token id) -> Option<u64> {
  for (int i = this->locals.locals_count - 1; i >= 0; i--) {
    Local* local = &this->locals.locals[i];
    if (local->id.IdentifiersEqual(id)) {
      return i;
    }
  }

  return OptionType::None;
}

fnc Compiler::FindLocal(Token id, ScopedLocals* scope) -> Option<u64> {
  return 0;
}

fnc Compiler::FindUpvalue(Token id) -> Option<u64> {
  if (this->curr_func->next == nullptr) return OptionType::None;

  return OptionType::None;
}

fnc Compiler::FindGlobal(Token id) -> Option<u64> {
  return this->global_pool->Find(id.len, id.start);
}

/*
 * Variables work like this:
 * GlobalPool is backed by a hash table, keyed by variable name
 * the value is an index into a buffer of Objects. The hash table
 * does not exist at runtime.
 *
 * Locals are stored directly into a Chunk's locals array
 * A Local's name is erased at runtime @TODO(eddie) - this bad from a debugging POV
 * Lookups are handled in Compiler, we store an array of names
 * here, and lookups return an index into the locals array
*/
fnc Compiler::LoadVariable(bool assignment) -> void {
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
    get = OpCode::GetGlobal;
    set = OpCode::SetGlobal;
  } else if (idx = this->FindUpvalue(this->prev); !idx.IsNone()) {
    get = OpCode::GetUpvalue;
    get = OpCode::SetUpvalue;
  } else {
    this->ErrorAtCurr("Undefined variable");
    return;
  }

  if (this->Match(Token::Lexeme::Equal) && assignment) {
    this->Expression();

    this->Emit(set);
  } else {
    this->Emit(get);
  }

  auto unwrapped = idx.Get();
  // @FIXME(eddie) - this is 8 bytes
  this->Emit(IntToBytes(&unwrapped), 4);
}

fnc Compiler::Emit(u8 byte) -> void {
  this->CurrentChunk()->AddInstruction(byte, this->prev.line);
}

fnc Compiler::Emit(u8* bytes, u32 count) -> void {
  this->CurrentChunk()->AddInstruction(bytes, count, this->prev.line);
}

fnc Compiler::Emit(OpCode op) -> void {
  u8 byte = static_cast<u8>(op);
  this->CurrentChunk()->AddInstruction(byte, this->prev.line);
}

fnc Compiler::Consume(Token::Lexeme type, const char* message) -> void {
  if (this->curr.type == type) {
    this->Advance();

    return;
  }

  this->ErrorAtCurr(message);
}

fnc Compiler::EndCompilation() -> void { this->Emit(OpCode::Return); }

fnc Compiler::GetParseRule(Token::Lexeme token) -> const ParseRule* {
  return &this->PARSE_RULES.at(token);
}

fnc Compiler::GetPrecedence(Precedence precedence) -> void {
  this->Advance();
  if (this->prev.type == Token::Lexeme::Return) {
    this->Advance();
  }

  const ParseRule* rule = this->GetParseRule(this->prev.type);

  if (rule->prefix == nullptr) {
    this->ErrorAtCurr("Expected expression");
    return;
  }

  u32 assignment_prec = static_cast<u32>(Precedence::Assignment);
  u32 curr_prec = static_cast<u32>(precedence);
  bool assign = curr_prec <= assignment_prec;

  rule->prefix(this, assign);

  while (precedence <=
         this->GetParseRule(this->curr.type)->precedence) {
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
fnc static Grammar::Number(Compiler* compiler, bool assign) -> void {
  f64 value = strtod(compiler->prev.start, nullptr);
  compiler->CurrentChunk()->AddLocal(Value(value), compiler->prev.line);
}

fnc static Grammar::Parenthesis(Compiler* compiler, bool assign) -> void {
  compiler->Expression(true);
  compiler->Consume(Token::Lexeme::RightParens, "Expect ')' after expression");
}

fnc static Grammar::Unary(Compiler* compiler, bool assign) -> void {
  Token::Lexeme op = compiler->prev.type;
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

fnc static Grammar::Binary(Compiler* compiler, bool assign) -> void {
  Token::Lexeme op = compiler->prev.type;

  int higher = static_cast<int>(Precedence::Term) + 1;
  auto next_higher = static_cast<Precedence>(higher);
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

fnc static Grammar::Literal(Compiler* compiler, bool assign) -> void {
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

fnc static Grammar::String(Compiler* compiler, bool assign) -> void {
  // skip the closing quote
  u32 length = compiler->prev.len - 2;
  char* start = const_cast<char*>(compiler->prev.start + 1);

  u32 index = compiler->string_pool->Alloc(length, start);

  compiler->Emit(OpCode::String);
  compiler->Emit(IntToBytes(&index), 4);
}

fnc static Grammar::Variable(Compiler* compiler, bool assign) -> void {
  compiler->LoadVariable(assign);
}

fnc static Grammar::AndOp(Compiler* compiler, bool assign) -> void {
  u32 short_circuit = compiler->Jump(OpCode::JumpFalse);

  compiler->Emit(OpCode::Pop);
  compiler->GetPrecedence(Precedence::And);
  compiler->PatchJump(short_circuit);
}

fnc static Grammar::OrOp(Compiler* compiler, bool assign) -> void {
  u32 short_circuit = compiler->Jump(OpCode::JumpTrue);

  compiler->Emit(OpCode::Pop);
  compiler->GetPrecedence(Precedence::Or);
  compiler->PatchJump(short_circuit);
}

fnc static Grammar::InvokeOp(Compiler* compiler, bool assign) -> void {
  u32 arg_count = 0;
  if (compiler->curr.type != Token::Lexeme::RightParens) {
    do {
      compiler->Expression(true);
      arg_count++;
    } while (compiler->Match(Token::Lexeme::Comma));
  }

  compiler->Consume(Token::Lexeme::RightParens, "Expected closing parenthesis ')' after arguments");

  compiler->Emit(OpCode::Invoke);
  compiler->Emit(IntToBytes(&arg_count), 4);
}
