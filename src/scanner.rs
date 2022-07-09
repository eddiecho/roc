use std::iter::{Iterator, Peekable};
use std::str::Chars;

use crate::token::{Token, Lexeme};

fn is_digit(c: char) -> bool {
    c >= '0' && c <= '9'
}

fn is_whitespace(c: char) -> bool {
    c == ' ' || c == '\n' || c == '\t' || c == '\r'
}

fn is_identifier_char(c: char) -> bool {
    c == '(' || c == ')' || c == '{' || c == '}' || c == '=' ||
    c == '[' || c == ']' || c == '/' || c == '+' || c == '-' ||
    c == '*' || c == ',' || c == '.' || c == ';' || c == ':' ||
    c == '>' || c == '<' || c == '|' || c == '\'' || c == '"'
}

pub struct Scanner<'a> {
    source: Peekable<Chars<'a>>,
    curr_idx: u32,
    curr_col: u32,
    curr_line: u32,
    start_idx: u32,
}

impl Scanner<'_> {
    pub fn new(source: &str) -> Scanner {
        Scanner {
            source: source.chars().peekable(),
            curr_col: 0,
            curr_idx: 0,
            curr_line: 1,
            start_idx: 0,
        }
    }

    fn end(&mut self) -> bool {
        match self.source.peek() {
            None => true,
            Some(_) => false,
        }
    }

    fn next(&mut self) -> Option<char> {
        self.curr_idx += 1;
        self.curr_col += 1;
        self.source.next()
    }

    fn match_next(&mut self, expected: char) -> bool {
        match self.source.peek() {
            None => return false,
            Some(next) => {
                if expected != *next {
                    return false;
                }

                self.curr_idx += 1;
                self.source.next();
                return true;
            }
        }
    }

    fn make_token(&self, variant: Lexeme) -> Token {
        Token {
            variant,
            idx: self.curr_idx,
            col: self.curr_col,
            line: self.curr_line,
            len: self.curr_idx - self.start_idx,
        }
    }

    fn skip_whitespace(&mut self) {
        loop {
            match self.source.peek() {
                None => return,
                Some(c) => {
                    match c {
                        ' ' | '\t' |'\r' => {
                            self.next();
                        }
                        '\n' => {
                            self.curr_line += 1;
                            self.curr_col = 0;
                            self.next();
                        }
                        _ => return
                    }
                }
            }
        }
    }

    fn match_string(&mut self, expected: char) -> Lexeme {
        loop {
            match self.source.peek() {
                None => break,
                Some(c) => {
                    if *c == expected {
                        break;
                    }

                    if *c == '\n' {
                        self.curr_line += 1;
                    }

                    self.next();
                }
            }
        }

        if self.end() {
            return Lexeme::Error("Unterminated string".to_string());
        }

        // the closing quote
        self.next();
        Lexeme::String
    }

    fn match_number(&mut self) -> Lexeme {
        loop {
            match self.source.peek() {
                None => break,
                Some(c) => {
                    if is_digit(*c) {
                        self.next();
                    } else {
                        break;
                    }
                }
            }
        }

        // holy fuck this is awful
        match self.source.peek() {
            None => {},
            Some(c) => {
                if *c == '.' {
                    match self.source.nth(1) {
                        None => {},
                        Some(c1) => {
                            match c1 {
                                '0'..='9' => {
                                    self.next();
                                    self.next();

                                    loop {
                                        match self.source.peek() {
                                            None => break,
                                            Some(c2) => {
                                                if is_digit(*c2) {
                                                    self.next();
                                                } else {
                                                    break;
                                                }
                                            }
                                        }
                                    }
                                }
                                _ => {}
                            }
                        }
                    }
                }
            }
        }

        Lexeme::Number
    }

    fn check_keyword(&mut self, keyword: &str, remaining: Peekable<Chars<'_>>, expected: Lexeme) -> Lexeme {
        let mut ret = expected;
        let mut identifier = remaining.take_while(|c| {
            !is_identifier_char(*c) && !is_whitespace(*c)
        }).into_iter().peekable();

        for c in keyword.chars() {
            match identifier.next() {
                None => ret = Lexeme::Identifier,
                Some(next) => {
                    self.next();
                    if c != next {
                        ret = Lexeme::Identifier;
                    }
                }
            }
        }

        match identifier.peek() {
            None => {},
            Some(_) => ret = Lexeme::Identifier,
        }

        for _ in 0..identifier.count() {
            self.next();
        }

        ret
    }

    fn skip_identifier(&mut self) {
        loop {
            match self.source.peek() {
                None => return,
                Some(c) => {
                    if is_identifier_char(*c) || is_whitespace(*c) {
                        return;
                    }

                    self.next();
                }
            }
        }
    }

    // TODO - this doesnt properly advance,
    // TODO - ideally, any utf8 codepoint should be valid identifier
    fn match_identifier(&mut self, first: char) -> Lexeme {
        let mut identifier = self.source.clone();

        match first {
            'a' => self.check_keyword("nd", identifier, Lexeme::And),
            'd' => self.check_keyword("efer", identifier, Lexeme::Defer),
            'e' => self.check_keyword("lse", identifier, Lexeme::Else),
            'f' => {
                match self.source.peek() {
                    None => {
                        self.skip_identifier();
                        return Lexeme::Identifier;
                    }
                    Some(second) => {
                        identifier.next();
                        match second {
                            'a' => self.check_keyword("lse", identifier, Lexeme::False),
                            'o' => self.check_keyword("r", identifier, Lexeme::For),
                            'u' => self.check_keyword("n", identifier, Lexeme::Function),
                            _ => {
                                self.skip_identifier();
                                return Lexeme::Identifier;
                            }
                        }
                    }
                }
            }
            'i' => self.check_keyword("f", identifier, Lexeme::If),
            'o' => self.check_keyword("r", identifier, Lexeme::Or),
            'r' => self.check_keyword("eturn", identifier, Lexeme::Return),
            's' => self.check_keyword("truct", identifier, Lexeme::Struct),
            't' => self.check_keyword("rue", identifier, Lexeme::True),
            _ => {
                self.skip_identifier();
                return Lexeme::Identifier;
            }
        }
    }

    pub fn advance(&mut self) -> Token {
        self.start_idx = self.curr_idx;
        self.skip_whitespace();

        let mut variant = Lexeme::Eof;

        match self.next() {
            Some(c) => {
                match c {
                    '(' => variant = Lexeme::LeftParens,
                    ')' => variant = Lexeme::RightParens,
                    '{' => variant = Lexeme::LeftBrace,
                    '}' => variant = Lexeme::RightBrace,
                    ';' => variant = Lexeme::SemiColon,
                    ',' => variant = Lexeme::Comma,
                    '.' => variant = Lexeme::Dot,
                    '+' => variant = if self.match_next('=') {
                        Lexeme::PlusEqual
                    } else {
                        Lexeme::Plus
                    },
                    '-' => variant = if self.match_next('=') {
                        Lexeme::MinusEqual
                    } else {
                        Lexeme::Minus
                    },
                    '*' => variant = if self.match_next('=') {
                        Lexeme::StarEqual
                    } else {
                        Lexeme::Star
                    },
                    // TODO - need comment
                    '/' => variant = if self.match_next('=') {
                        Lexeme::SlashEqual
                    } else {
                        Lexeme::Slash
                    },
                    '!' => variant = if self.match_next('=') {
                        Lexeme::BangEqual
                    } else {
                        Lexeme::Bang
                    },
                    '=' => variant = if self.match_next('=') {
                        Lexeme::Equality
                    } else {
                        Lexeme::Assignment
                    },
                    '>' => variant = if self.match_next('=') {
                        Lexeme::GreaterEqual
                    } else {
                        Lexeme::Greater
                    },
                    '<' => variant = if self.match_next('=') {
                        Lexeme::LessEqual
                    } else {
                        Lexeme::Less
                    },
                    '"' | '\'' => {
                        variant = self.match_string(c);
                    }
                    '0'..='9' => {
                        variant = self.match_number();
                    }
                    _ => variant = self.match_identifier(c),
                }
            }
            None => {},
        }

        self.make_token(variant)
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_scanner() {
        const INPUT: &'static str = "
() // single line comment
/*
multi line \
comment
 */ 1234 fun
    ";

        let mut test = vec![];
        let mut scanner = Scanner::new(INPUT);

        loop {
            let token = scanner.advance();
            println!("{:?}", token);
            test.push(token.clone());

            match token.variant {
                Lexeme::Eof => break,
                _ => continue,
            }
        }

        let compare = vec![
            Lexeme::LeftParens,
            Lexeme::RightParens,
            Lexeme::Comment,
            Lexeme::Comment,
            Lexeme::Number,
            Lexeme::Eof,
        ];

        for (t1, t2) in compare.into_iter().zip(test) {
            assert!(t1 == t2.variant);
        }
    }
}
