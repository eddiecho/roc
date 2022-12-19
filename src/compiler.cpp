#pragma once

#include "compiler.h"

#include <stdio.h>
#include <cstdarg>
#include <cstring>

#include "utils/utils.h"
#include "common.h"
#include "value.h"

Token::Token() noexcept {
  this->type = Token::Lexeme::Eof;
  this->start = nullptr;
  this->len = 0;
  this->line = 0;
}

Token::Token(Token::Lexeme type, const char* start, u32 len, u32 line) noexcept :
  type{type}, start{start}, len{len}, line{line}
{}

// @TODO(eddie) - I don't like this. Should be able to point to where the error starts
// @STDLIB
Token::Token(const char* error) noexcept {
  this->type = Token::Lexeme::Error;
  this->start = error;
  this->len = (u32)strlen(error);
  this->line = 0;
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
  return Token {
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
      default: return;
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
  while (Utils::IsDigit(this->Peek())) this->Pop();

  // you get one .
  if (this->Peek() == '.' && Utils::IsDigit(this->PeekNext())) {
    this->Pop();

    while (Utils::IsDigit(this->Peek())) this->Pop();
  }

  return this->MakeToken(Token::Lexeme::Number);
}

// @STDLIB
auto Scanner::CheckKeyword(u32 start, u32 length, const char* rest, Token::Lexeme possible) const
  -> const Token::Lexeme {
  if (this->curr - this->start == start + length &&
      memcmp(this->start + start, rest, length) == 0) {
    return possible;
  }

  return Token::Lexeme::Identifier;
}

auto Scanner::IdentifierType() const -> const Token::Lexeme {
  switch (*this->start) {
    case 'a': return this->CheckKeyword(1, 2, "nd", Token::Lexeme::And);
    case 'e': return this->CheckKeyword(1, 3, "lse", Token::Lexeme::Else);
    case 'f': {
      if (this->curr - this->start > 1) {
        switch (this->start[1]) {
          case 'a': return this->CheckKeyword(2, 3, "lse", Token::Lexeme::False);
          case 'o': return this->CheckKeyword(2, 1, "r", Token::Lexeme::For);
          case 'u': return this->CheckKeyword(2, 1, "n", Token::Lexeme::Function);
        }
      }
      break;
    }
    case 'i': return this->CheckKeyword(1, 1, "f", Token::Lexeme::If);
    case 'o': return this->CheckKeyword(1, 1, "r", Token::Lexeme::Or);
    case 'r': return this->CheckKeyword(1, 5, "eturn", Token::Lexeme::Return);
    case 's': return this->CheckKeyword(1, 5, "truct", Token::Lexeme::Struct);
    case 't': return this->CheckKeyword(1, 3, "rue", Token::Lexeme::True);
    case 'v': return this->CheckKeyword(1, 2, "ar", Token::Lexeme::Var);
    case 'w': return this->CheckKeyword(1, 4, "hile", Token::Lexeme::While);
  }

  return Token::Lexeme::Identifier;
}

auto Scanner::IdentifierToken() const -> const Token {
  return this->MakeToken(Token::Lexeme::Identifier);
}

auto Scanner::ScanToken() -> Token {
  this->start = this->curr;

  this->SkipWhitespace();

  if (this->IsEnd()) {
    return Token();
  }

  char c = this->Pop();
  switch (c) {
    case '(': return this->MakeToken(Token::Lexeme::LeftParens);
    case ')': return this->MakeToken(Token::Lexeme::RightParens);
    case '{': return this->MakeToken(Token::Lexeme::LeftBrace);
    case '}': return this->MakeToken(Token::Lexeme::RightBrace);
    case ';': return this->MakeToken(Token::Lexeme::Semicolon);
    case ':': return this->MakeToken(Token::Lexeme::Colon);
    case ',': return this->MakeToken(Token::Lexeme::Colon);
    case '.': return this->MakeToken(Token::Lexeme::Dot);
    case '-': return this->MakeToken(Token::Lexeme::Minus);
    case '+': return this->MakeToken(Token::Lexeme::Plus);
    case '*': return this->MakeToken(Token::Lexeme::Star);
    case '!': return this->MakeToken(
                  this->Match('=') ? Token::Lexeme::BangEqual : Token::Lexeme::Bang);
    case '=': return this->MakeToken(
                  this->Match('=') ? Token::Lexeme::EqualEqual : Token::Lexeme::Equal);
    case '<': return this->MakeToken(
                  this->Match('=') ? Token::Lexeme::LessEqual : Token::Lexeme::Less);
    case '>': return this->MakeToken(
                  this->Match('=') ? Token::Lexeme::GreaterEqual : Token::Lexeme::Greater);
    // @TODO(eddie) - Multiline comments
    case '/': {
      if (this->PeekNext() == '/') {
        while (this->Peek() != '\n' && !this->IsEnd()) this->Pop();
        return this->MakeToken(Token::Lexeme::Comment);
      } else {
        return this->MakeToken(Token::Lexeme::Slash);
      }
    }
    case '"': return this->StringToken();
    case '0':
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
    case '8':
    case '9': return this->NumberToken();
    // it's the wild west here baby
    default: return this->IdentifierToken();
  }
}

Compiler::Compiler() noexcept {
  this->scanner = new Scanner();
  this->parser = new Parser();
}

std::unordered_map<Token::Lexeme, ParseRule> Compiler::PARSE_RULES = {
  {Token::Lexeme::LeftParens, ParseRule(&Grammar::Parenthesis, nullptr, Precedence::None)},
  {Token::Lexeme::RightParens, ParseRule(nullptr, nullptr, Precedence::None)},

  {Token::Lexeme::Minus, ParseRule(&Grammar::Unary, &Grammar::Binary, Precedence::Term)},
  {Token::Lexeme::Plus, ParseRule(nullptr, &Grammar::Binary, Precedence::Term)},
  {Token::Lexeme::Slash, ParseRule(nullptr, &Grammar::Binary, Precedence::Factor)},
  {Token::Lexeme::Star, ParseRule(nullptr, &Grammar::Binary, Precedence::Factor)},

  {Token::Lexeme::Number, ParseRule(&Grammar::Number, nullptr, Precedence::None)},

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
  {Token::Lexeme::Bang, ParseRule(nullptr, nullptr, Precedence::None)},
  {Token::Lexeme::BangEqual, ParseRule(nullptr, nullptr, Precedence::None)},
  {Token::Lexeme::Equal, ParseRule(nullptr, nullptr, Precedence::None)},
  {Token::Lexeme::EqualEqual, ParseRule(nullptr, nullptr, Precedence::None)},
  {Token::Lexeme::Greater, ParseRule(nullptr, nullptr, Precedence::None)},
  {Token::Lexeme::GreaterEqual, ParseRule(nullptr, nullptr, Precedence::None)},
  {Token::Lexeme::Less, ParseRule(nullptr, nullptr, Precedence::None)},
  {Token::Lexeme::LessEqual, ParseRule(nullptr, nullptr, Precedence::None)},
  {Token::Lexeme::Identifier, ParseRule(nullptr, nullptr, Precedence::None)},
  {Token::Lexeme::String, ParseRule(nullptr, nullptr, Precedence::None)},
  {Token::Lexeme::And, ParseRule(nullptr, nullptr, Precedence::None)},
  {Token::Lexeme::Else, ParseRule(nullptr, nullptr, Precedence::None)},
  {Token::Lexeme::False, ParseRule(nullptr, nullptr, Precedence::None)},
  {Token::Lexeme::For, ParseRule(nullptr, nullptr, Precedence::None)},
  {Token::Lexeme::Function, ParseRule(nullptr, nullptr, Precedence::None)},
  {Token::Lexeme::If, ParseRule(nullptr, nullptr, Precedence::None)},
  {Token::Lexeme::Or, ParseRule(nullptr, nullptr, Precedence::None)},
  {Token::Lexeme::Return, ParseRule(nullptr, nullptr, Precedence::None)},
  {Token::Lexeme::Struct, ParseRule(nullptr, nullptr, Precedence::None)},
  {Token::Lexeme::True, ParseRule(nullptr, nullptr, Precedence::None)},
  {Token::Lexeme::Var, ParseRule(nullptr, nullptr, Precedence::None)},
  {Token::Lexeme::While, ParseRule(nullptr, nullptr, Precedence::None)},

};

auto Compiler::Init(const char* src, Chunk* chunk) -> void {
  this->chunk = chunk;
  this->scanner->Init(src);
}

auto Compiler::Expression() -> void {
  this->GetPrecedence(Precedence::Assignment);
}

auto Compiler::Compile() -> bool {

  this->Advance();
  this->Expression();
  this->Consume(Token::Lexeme::Eof, "End of file, expected expression");
  this->EndCompilation();

  return !this->parser->state.error;
}

auto Compiler::Advance() -> void {
  u32 line = 0xFFFFFFFF;
  Token token;
  this->parser->prev = this->parser->curr;

  while (1) {
    token = this->scanner->ScanToken();
    this->parser->curr = token;

    // output debug token info
    if (token.line != line) {
      printf("%4d ", token.line);
    } else {
      printf("   | ");
    }
    printf("%s '%.*s'\n", token.Type(), token.len, token.start);

    if (this->parser->curr.type != Token::Lexeme::Error) return;

    this->parser->ErrorAtCurr(this->parser->curr.start);
  }
}

Parser::Parser() noexcept {
  this->curr = Token();
  this->prev = Token();
}

auto Parser::ErrorAtCurr(const char* message) -> void {
  if (this->state.panic) return;

  fprintf(stderr, "[line %d] Error", this->curr.line);
  fprintf(stderr, " at '%.*s'", this->curr.len, this->curr.start);
  fprintf(stderr, ": %s\n", message);

  this->state.panic = 1;
  this->state.error = 1;
}

auto Compiler::Emit(u8 byte) -> void {
  this->chunk->AddChunk(byte, this->parser->prev.line);
}

auto Compiler::Emit(OpCode op) -> void {
  u8 byte = static_cast<u8>(op);
  this->chunk->AddChunk(byte, this->parser->prev.line);
}

auto Compiler::Consume(Token::Lexeme type, const char* message) -> void {
  if (this->parser->curr.type == type) {
    this->Advance();

    return;
  }

  this->parser->ErrorAtCurr(message);
}

auto Compiler::EndCompilation() -> void {
  this->Emit(OpCode::Return);
}

auto Compiler::GetParseRule(Token::Lexeme token) -> ParseRule* {
  return &this->PARSE_RULES[token];
}

auto Compiler::GetPrecedence(Precedence precedence) -> void {
  this->Advance();
  ParseRule* rule = this->GetParseRule(this->parser->prev.type);

  if (rule->prefix == nullptr) {
    this->parser->ErrorAtCurr("Expected expression");
    return;
  }

  rule->prefix(this);

  while (precedence <= this->GetParseRule(this->parser->curr.type)->precedence) {
    this->Advance();
    ParseRule* prev_rule = this->GetParseRule(this->parser->prev.type);
    prev_rule->infix(this);
  }
}

// @STDLIB
auto Grammar::Number(Compiler* compiler) -> void {
  f64 value = strtod(compiler->parser->prev.start, NULL);
  compiler->chunk->AddConstant(Value(value), compiler->parser->prev.line);
}

auto Grammar::Parenthesis(Compiler* compiler) -> void {
  compiler->Expression();
  compiler->Consume(Token::Lexeme::RightParens, "Expect ')' after expression");
}

auto Grammar::Unary(Compiler* compiler) -> void {
  Token::Lexeme op = compiler->parser->prev.type;
  compiler->GetPrecedence(Precedence::Unary);

  switch (op) {
    default: return;
    case Token::Lexeme::Minus: {
      compiler->Emit(OpCode::Negate);
      break;
    }
  }
}

auto Grammar::Binary(Compiler* compiler) -> void {
  Token::Lexeme op = compiler->parser->prev.type;

  int higher = static_cast<int>(Precedence::Term) + 1;
  Precedence next_higher = static_cast<Precedence>(higher);
  compiler->GetPrecedence(next_higher);

  switch (op) {
    default: return;
    case Token::Lexeme::Plus: { compiler->Emit(OpCode::Add); break; }
    case Token::Lexeme::Minus: { compiler->Emit(OpCode::Subtract); break; }
    case Token::Lexeme::Star: { compiler->Emit(OpCode::Multiply); break; }
    case Token::Lexeme::Slash: { compiler->Emit(OpCode::Divide); break; }
  }
}
