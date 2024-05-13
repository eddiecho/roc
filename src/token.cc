#include <assert.h>
#include <ctype.h>
#include <string.h>

#include "token.h"
#include "utils.h"

using namespace roc;

func constexpr static inline IsIdentifier(char c) noexcept -> bool {
  return !isspace(c) && !ispunct(c);
}

func constexpr static inline IsNumeric(char c) noexcept -> bool {
  return c >= '0' && c <= '9';
}

func inline Lexer::IsEnd() const noexcept -> bool {
  return *this->pos == 0;
}

func inline Lexer::Pop() noexcept -> char {
  this->col++;
  return *this->pos++;
}

func inline Lexer::Peek() const noexcept -> char {
  return *this->pos;
}

func inline Lexer::SkipWhitespace() noexcept -> void {
  while (true) {
    switch (this->Peek()) {
      default: return;
      case ' ':
      case '\r':
      case '\t': {
        this->Pop();
        break;
      }
      case '\n': {
        this->line++;
        this->col = 0;
        this->Pop();
        break;
      }
    }
  }
}

func Lexer::NumberToken() noexcept -> Token {
  u64 len = 1;

  while (IsNumeric(this->Peek())) {
    len++;
    this->Pop();
  }

  return Token {
    .type = Lexeme::Number,
    .start = this->start,
    .len = len,
  };
}

func Lexer::MatchKeyword(u64 prefix, u64 len, const char* test, Lexeme possible) noexcept -> Lexeme {
  auto range = static_cast<u64>(this->pos - this->start);
  auto keylen = prefix + len;

  if (range == keylen) {
    if (memcmp(this->start + prefix, test, len) == 0) return possible;
  }

  return Lexeme::Identifier;
}

func Lexer::IdentifierToken(char curr) noexcept -> Token {
  u64 len = 1;
  while (IsIdentifier(this->Peek())) {
    len++;
    this->Pop();
  }

  auto type = Lexeme::Identifier;
  switch (curr) {
    case 'f': {
      auto constexpr key = utils::len("fn") - 1;
      type = this->MatchKeyword(1, key, "n", Lexeme::Function);
      break;
    }
    case 'l': {
      auto constexpr key = utils::len("let") - 1;
      type = this->MatchKeyword(1, key, "et", Lexeme::Let);
      break;
    }
    case 'r': {
      auto constexpr key = utils::len("return") - 1;
      type = this->MatchKeyword(1, key, "eturn", Lexeme::Return);
      break;
    }
  }

  return Token {
    .type = type,
    .start = this->pos,
    .len = len,
  };
}

func Lexer::NextToken() noexcept -> Token {
  this->SkipWhitespace();
  this->start = this->pos;

  Token ret = {
    .type = Lexeme::Eof,
    .start = this->start,
    .len = 1,
  };

  if (this->IsEnd()) {
    return ret;
  }

  auto c = this->Pop();

  switch (c) {
    default: {
      return this->IdentifierToken(c);
    }
    case '=': {
      ret.type = Lexeme::Equal;
      break;
    }
    case '+': {
      ret.type = Lexeme::Add;
      break;
    }
    case ';': {
      ret.type = Lexeme::Semicolon;
      break;
    }
    case ',': {
      ret.type = Lexeme::Comma;
      break;
    }
    case '(': {
      ret.type = Lexeme::LeftParens;
      break;
    }
    case ')': {
      ret.type = Lexeme::RightParens;
      break;
    }
    case '{': {
      ret.type = Lexeme::LeftBrace;
      break;
    }
    case '}': {
      ret.type = Lexeme::RightBrace;
      break;
    }
    case '0':
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
    case '8':
    case '9': {
      return this->NumberToken();
    }
  }

  return ret;
}
