#include <gtest/gtest.h>

#include "token.h"

using namespace roc;

TEST(TokenTest, Sanity) {
  const char* input = "=+(){},;";

  const Lexeme tests[] = {
    Lexeme::Equal,
    Lexeme::Add,
    Lexeme::LeftParens,
    Lexeme::RightParens,
    Lexeme::LeftBrace,
    Lexeme::RightBrace,
    Lexeme::Comma,
    Lexeme::Semicolon,
    Lexeme::Eof,
  };

  Lexer lexer {
    .start = input,
    .pos = input,
  };

  for (auto test : tests) {
    auto token = lexer.NextToken();

    EXPECT_EQ(token.type, test);
  }
}

TEST(TokenTest, Simple) {
  const char* input =
"let five = 5;\n"
"let ten = 10;\n"
"fn add(x, y) {\n"
"  return x + y;\n"
"}\n"
"let result = add(five, ten);";

  const Lexeme test[] = {
    Lexeme::Let,
    Lexeme::Identifier,
    Lexeme::Equal,
    Lexeme::Number,
    Lexeme::Semicolon,

    Lexeme::Let,
    Lexeme::Identifier,
    Lexeme::Equal,
    Lexeme::Number,
    Lexeme::Semicolon,

    Lexeme::Function,
    Lexeme::Identifier,
    Lexeme::LeftParens,
    Lexeme::Identifier,
    Lexeme::Comma,
    Lexeme::Identifier,
    Lexeme::RightParens,
    Lexeme::LeftBrace,

    Lexeme::Return,
    Lexeme::Identifier,
    Lexeme::Add,
    Lexeme::Identifier,
    Lexeme::Semicolon,

    Lexeme::RightBrace,

    Lexeme::Let,
    Lexeme::Identifier,
    Lexeme::Equal,
    Lexeme::Identifier,
    Lexeme::LeftParens,
    Lexeme::Identifier,
    Lexeme::Comma,
    Lexeme::Identifier,
    Lexeme::RightParens,
    Lexeme::Semicolon,
    Lexeme::Eof,
  };

  Lexer lexer {
    .start = input,
    .pos = input,
  };

  size_t len = sizeof(test) / sizeof(test[0]);
  for (size_t i = 0; i < len; i++) {
    auto t = test[i];
    auto token = lexer.NextToken();

    EXPECT_EQ(token.type, t);
  }
}
