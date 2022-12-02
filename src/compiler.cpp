#pragma once

#include "compiler.h"

#include <cstring>
#include <stdio.h>

#include "utils/utils.h"

Token::Token() noexcept {
  this->type = Token::Lexeme::Eof;
  this->start = nullptr;
  this->len = 0;
  this->line = 0;
}

Token::Token(Token::Lexeme type, const char* start, u32 len, u32 line) noexcept :
  type{type}, start{start}, len{len}, line{line}
{}

// @TODO - I don't like this. Should be able to point to where the error starts
Token::Token(const char* error) {
  this->type = Token::Lexeme::Error;
  this->start = error;
  this->len = strlen(error);
  this->line = 0;
}

auto constexpr inline Scanner::IsEnd() -> bool {
  return *this->curr == '\0';
}

auto inline Scanner::Advance() -> char {
  this->curr++;
  this->row++;
  return this->curr[-1];
}

auto inline Scanner::MakeToken(Token::Lexeme type) -> Token {
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

auto constexpr inline Scanner::Peek() -> char {
  return *this->curr;
}

auto constexpr inline Scanner::PeekNext() -> char {
  return this->IsEnd() ? '\0' : this->curr[1];
}

auto Scanner::SkipWhitespace() -> void {
  while (1) {
    switch(this->Peek()) {
      case ' ':
      case '\r':
      case '\t': {
        this->Advance();
        break;
      }
      case '\n': {
        this->line++;
        this->row = 0;
        this->Advance();
        break;
      }
      default: return;
    }
  }
}

auto Scanner::StringToken() -> Token {
  while (this->Peek() != '"' && !this->IsEnd()) {
    if (this->Peek() == '\n') this->line++;
    this->Advance();
  }

  if (this->IsEnd()) return Token("Unterminated string");

  this->Advance();
  return this->MakeToken(Token::Lexeme::String);
}

auto Scanner::NumberToken() -> Token {
  while (Utils::IsDigit(this->Peek())) this->Advance();

  // you get one .
  if (this->Peek() == '.' && Utils::IsDigit(this->PeekNext())) {
    this->Advance();

    while (Utils::IsDigit(this->Peek())) this->Advance();
  }

  return this->MakeToken(Token::Lexeme::Number);
}

auto Scanner::CheckKeyword(u32 start, u32 length, const char* rest, Token::Lexeme possible) -> Token::Lexeme {
  if (this->curr - this->start == start + length &&
      memcmp(this->start + start, rest, length) == 0) {
    return possible;
  }

  return Token::Lexeme::Identifier;
}

auto Scanner::IdentifierType() -> Token::Lexeme {
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

auto Scanner::IdentifierToken() -> Token {
  return this->MakeToken(Token::Lexeme::Identifier);
}

auto Scanner::ScanToken() -> Token {
  this->start = this->curr;

  if (this->IsEnd()) {
    return Token();
  }

  char c = this->Advance();
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
    // @TODO - Multiline comments
    case '/': {
      if (this->PeekNext() == '/') {
        while (this->Peek() != '\n' && !this->IsEnd()) this->Advance();
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

auto Compiler::Compile(const char* src) -> void {
  int line = -1;

  while (1) {
    Token token = this->scanner->ScanToken();

    if (token.line != line) {
      printf("%4d ", token.line);
    } else {
      printf("   | ");
    }
    printf("%s '%.*s'\n", token.Type(), token.len, token.start);

    if (token.IsEnd()) break;
  }
}
