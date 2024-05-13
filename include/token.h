#pragma once

#include "utils.h"

namespace roc {

enum class Lexeme {
  Error,
  Eof,
  Identifier,
  Number,

  // misc operators
  Comma,
  Semicolon,
  LeftParens,
  RightParens,
  LeftBrace,
  RightBrace,

  // binary operators
  Equal,
  Add,

  // keywords
  Function,
  Let,
  Return,
};

struct Token {
  Lexeme type;
  const char* start;
  u64 len;
};

struct Lexer {
  const char* start = 0;
  const char* pos = 0;
  u64 line = 1;
  u64 col = 1;

  func NextToken() noexcept -> Token;

 private:
  func inline IsEnd() const noexcept -> bool;
  func inline SkipWhitespace() noexcept -> void;
  func inline Pop() noexcept -> char;
  func inline Peek() const noexcept -> char;
  func NumberToken() noexcept -> Token;
  func IdentifierToken(char) noexcept -> Token;
  func MatchKeyword(u64, u64, const char*, Lexeme) noexcept -> Lexeme;
};

}
