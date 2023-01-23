#include "compiler.h"

#include <ctype.h>
#include <stdio.h>
#include <string.h>

#include "common.h"
#include "global_pool.h"
#include "object.h"
#include "string_pool.h"
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
      return this->CheckKeyword(1, 1, "f", Token::Lexeme::If);
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
  this->start = this->curr;

  this->SkipWhitespace();

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
      return this->MakeToken(Token::Lexeme::Colon);
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

std::unordered_map<Token::Lexeme, ParseRule> Compiler::PARSE_RULES = {
    {Token::Lexeme::LeftParens,
     ParseRule(&Grammar::Parenthesis, nullptr, Precedence::None)},
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

    // @TODO(eddie)
    {Token::Lexeme::LeftBrace, ParseRule(nullptr, nullptr, Precedence::None)},
    {Token::Lexeme::RightBrace, ParseRule(nullptr, nullptr, Precedence::None)},
    {Token::Lexeme::Eof, ParseRule(nullptr, nullptr, Precedence::None)},
    {Token::Lexeme::Error, ParseRule(nullptr, nullptr, Precedence::None)},
    {Token::Lexeme::Comment, ParseRule(nullptr, nullptr, Precedence::None)},
    {Token::Lexeme::Comma, ParseRule(nullptr, nullptr, Precedence::None)},
    {Token::Lexeme::Dot, ParseRule(nullptr, nullptr, Precedence::None)},
    {Token::Lexeme::Semicolon, ParseRule(nullptr, nullptr, Precedence::None)},
    {Token::Lexeme::Colon, ParseRule(nullptr, nullptr, Precedence::None)},
    {Token::Lexeme::Equal, ParseRule(nullptr, nullptr, Precedence::None)},
    {Token::Lexeme::Identifier, ParseRule(nullptr, nullptr, Precedence::None)},
    {Token::Lexeme::And, ParseRule(nullptr, nullptr, Precedence::None)},
    {Token::Lexeme::Else, ParseRule(nullptr, nullptr, Precedence::None)},
    {Token::Lexeme::For, ParseRule(nullptr, nullptr, Precedence::None)},
    {Token::Lexeme::Function, ParseRule(nullptr, nullptr, Precedence::None)},
    {Token::Lexeme::If, ParseRule(nullptr, nullptr, Precedence::None)},
    {Token::Lexeme::Or, ParseRule(nullptr, nullptr, Precedence::None)},
    {Token::Lexeme::Return, ParseRule(nullptr, nullptr, Precedence::None)},
    {Token::Lexeme::Struct, ParseRule(nullptr, nullptr, Precedence::None)},
    {Token::Lexeme::Var, ParseRule(nullptr, nullptr, Precedence::None)},
    {Token::Lexeme::While, ParseRule(nullptr, nullptr, Precedence::None)},

};

fnc Compiler::Init(const char* src,
                   Chunk* chunk,
                   StringPool* string_pool,
                   GlobalPool* global_pool)
    -> void {
  this->chunk = chunk;
  this->string_pool = string_pool;
  this->global_pool = global_pool;
  this->scanner.Init(src);
}

fnc Compiler::Compile() -> bool {
  this->Advance();
  /*
  this->Expression();
  this->Consume(Token::Lexeme::Eof, "End of file, expected expression");
    */

  while (!this->Match(Token::Lexeme::Eof)) {
    this->Declaration();
  }

  this->EndCompilation();

  return !this->state.error;
}

fnc Compiler::Declaration() -> void {
  if (this->Match(Token::Lexeme::Var)) {
    this->VariableDeclaration();
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
  } else {
    // technically, expressions without a corresponding assignment or the above are called
    // "expression statements"
    // basically just calling a function and discarding the return value
    this->Expression();
    this->Consume(Token::Lexeme::Semicolon, "Expected semicolon ';' after an expression.");
//    this->Emit(OpCode::Pop);
  }
}

fnc Compiler::Expression() -> void {
  this->GetPrecedence(Precedence::Assignment);
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
  u32 global_var_idx = this->Identifier("Expected variable name");

  if (this->Match(Token::Lexeme::Equal)) {
    this->Expression();
  }

  this->Consume(Token::Lexeme::Semicolon, "Expected semicolon ';' after variable declaration");

  this->Emit(OpCode::SetGlobal);
  this->Emit(reinterpret_cast<u8*>(&global_var_idx), 4);
}

fnc Compiler::Identifier(const char* err) -> u32 {
  this->Consume(Token::Lexeme::Identifier, err);

  // @TODO(eddie) - globals only for now
  return this->global_pool->Alloc(this->prev.len, this->prev.start);
}

fnc Compiler::LoadVariable(bool assignment) -> void {
  u32 global_var_idx = this->Identifier("Expected variable name");

  if (this->Match(Token::Lexeme::Equal) && assignment) {
    this->Expression();

    this->Emit(OpCode::SetGlobal);
    this->Emit(reinterpret_cast<u8*>(&global_var_idx), 4);
  } else {
    if (global_var_idx == GlobalPool::INVALID_INDEX) {
      this->ErrorAtCurr("Undefined global variable");
    } else {
      this->Emit(OpCode::LoadGlobal);
      this->Emit(reinterpret_cast<u8*>(&global_var_idx), 4);
    }
  }
}

fnc Compiler::Emit(u8 byte) -> void {
  this->chunk->AddChunk(byte, this->prev.line);
}

fnc Compiler::Emit(u8* bytes, u32 count) -> void {
  this->chunk->AddChunk(bytes, count, this->prev.line);
}

fnc Compiler::Emit(OpCode op) -> void {
  u8 byte = static_cast<u8>(op);
  this->chunk->AddChunk(byte, this->prev.line);
}

fnc Compiler::Consume(Token::Lexeme type, const char* message) -> void {
  if (this->curr.type == type) {
    this->Advance();

    return;
  }

  this->ErrorAtCurr(message);
}

fnc Compiler::EndCompilation() -> void { this->Emit(OpCode::Return); }

fnc Compiler::GetParseRule(Token::Lexeme token) -> ParseRule* {
  return &this->PARSE_RULES[token];
}

fnc Compiler::GetPrecedence(Precedence precedence) -> void {
  this->Advance();
  ParseRule* rule = this->GetParseRule(this->prev.type);

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
    ParseRule* prev_rule = this->GetParseRule(this->prev.type);
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
  compiler->chunk->AddConstant(Value(value), compiler->prev.line);
}

fnc static Grammar::Parenthesis(Compiler* compiler, bool assign) -> void {
  compiler->Expression();
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
  u8* index_bytes = reinterpret_cast<u8*>(&index);
  compiler->Emit(index_bytes, 4);
}

fnc static Grammar::Variable(Compiler* compiler, bool assign) -> void {
  compiler->LoadVariable(assign);
}
