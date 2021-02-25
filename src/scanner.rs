use crate::token::*;

use std::i64;
use std::str::Chars;
use std::vec::Vec;

pub struct Scanner<'a> {
    source: Chars<'a>,
    tokens: Vec<Token>,
    curr_row: u32,
    curr_col: u32,
}

#[repr(u32)]
enum Whitespace {
    General = 0x01,
    NewLine = 0x02,
}

fn is_whitespace(c: u8) -> u32 {
    let mut result: u32 = 0;

    let disc = ((c == b'\n') as u32) << 1;
    result |= disc & (Whitespace::NewLine as u32);
    let disc = (c == b'\t') as u32;
    result |= disc & (Whitespace::General as u32);
    let disc = (c == b'\r') as u32;
    result |= disc & (Whitespace::General as u32);
    let disc = (c == b' ') as u32;
    result |= disc & (Whitespace::General as u32);

    result
}

fn is_digit(c: char) -> bool {
    c >= '0' && c <= '9'
}

fn is_alpha(c: char) -> bool {
    (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || (c == '_')
}

fn is_alphanumeric(c: char) -> bool {
    is_alpha(c) || is_digit(c)
}

impl Scanner<'_> {
    pub fn new(source: &str) -> Scanner {
        Scanner {
            source: source.chars(),
            tokens: vec![],
            curr_col: 1,
            curr_row: 1,
        }
    }

    fn peek(&mut self) -> Option<char> {
        let c = self.source.nth(0)?;
        Some(c)
    }

    fn next(&mut self) -> Option<char> {
        let c = self.source.next()?;
        Some(c)
    }

    fn good(&self) -> bool {
        self.source.as_str().is_empty()
    }

    fn skip_whitespace(&mut self) {
        while let Some(c) = self.next() {
            let whitespace = is_whitespace(c as u8);
            match whitespace {
                0x01 => {
                    self.curr_col += 1;
                },
                0x02 => {
                    self.curr_row += 1;
                    self.curr_col = 1;
                },
                _ => {
                    break;
                }
            }
        }
    }

    // TODO - does this work if c is \n?
    fn match_char(&mut self, c: char) -> bool {
        let res = match self.peek() {
            None => false,
            Some(cc) => cc == c,
        };
        self.curr_col += res as u32;

        if res {
            self.next();
        }
        res
    }

    fn find_next(&mut self, c: char) {
        while let Some(byte) = self.next() {
            self.curr_col += 1;
            if byte == '\n' {
                self.curr_col = 1;
                self.curr_row += 1;
            }

            if byte == c {
                break;
            }
        }
    }

    fn match_string(&mut self, delimit: char) -> String {
        let mut val = String::new();

        while let Some(byte) = self.next() {
            self.curr_col += 1;
            let mut c = byte;

            if byte == delimit {
                break;
            }

            if byte == '\n' {
                self.curr_col = 1;
                self.curr_row += 1;
            } else if byte == '\\' {
                c = match self.next() {
                    // TODO ERROR - better error handling here
                    None => {
                        panic!("End of file while parsing string");
                    },
                    Some(escape) => {
                        match escape {
                            'n' => {
                                self.curr_row += 1;
                                self.curr_col = 1;
                                '\n'
                            },
                            _ => escape,
                        }
                    }
                }
            }

            val.push(c);
        }

        val
    }

    fn match_decimal(&mut self) -> String {
        let mut buff = String::new();
        while let Some(c) = self.next() {
            match c {
                '_' => {
                    continue;
                },
                '0'..='9' => {
                    buff.push(c);
                },
                _ => {
                    break;
                }
            }
        }

        buff
    }

    fn match_hex(&mut self) -> String {
        let mut buff = String::new();
        while let Some(c) = self.next() {
            match c {
                '_' => {
                    continue;
                },
                '0'..='9' | 'a'..='f' | 'A'..='F' => {
                    buff.push(c);
                },
                _ => {
                    break;
                }
            }
        }

        buff
    }

    fn match_numeric(&mut self, first: char) -> LiteralVariant {
        // floats may not be anything but decimal
        if first == '0' {
            match self.peek() {
                None => return LiteralVariant::Integer(0),
                Some(c) => {
                    match c {
                        'b' => {
                            self.next();
                            match i64::from_str_radix(self.match_decimal().as_str(), 2) {
                                Ok(num) => return LiteralVariant::Integer(num),
                                Err(e) => panic!(e)
                            }
                        },
                        'o' => {
                            self.next();
                            match i64::from_str_radix(self.match_decimal().as_str(), 8) {
                                Ok(num) => return LiteralVariant::Integer(num),
                                Err(e) => panic!(e)
                            }
                        },
                        'x' => {
                            self.next();
                            match i64::from_str_radix(self.match_hex().as_str(), 16) {
                                Ok(num) => return LiteralVariant::Integer(num),
                                Err(e) => panic!(e)
                            }
                        },
                        '0'..='9' | '_' | '.' | 'e' => {
                            match i64::from_str_radix(self.match_decimal().as_str(), 10) {
                                Ok(num) => return LiteralVariant::Integer(num),
                                Err(e) => panic!(e)
                            }
                        },
                        _ => return LiteralVariant::Integer(0)
                    }
                }
            }
        }

        LiteralVariant::Integer(0)
    }

    fn advance(&mut self) -> Token {
        self.skip_whitespace();

        let mut token = Token::default();
        token.col = self.curr_col;
        token.row = self.curr_row;

        match self.next() {
            None => {
                token
            }
            Some(byte) => {
                match byte {
                    '(' => {
                        token.variant = Lexeme::LeftParens;
                    },
                    ')' => {
                        token.variant = Lexeme::RightParens;
                    },
                    '/' => {
                        if self.match_char('/') {
                            // single line comment
                            token.variant = Lexeme::Comment;
                            self.find_next('\n');
                        } else if self.match_char('*') {
                            // multi line comment
                            token.variant = Lexeme::Comment;
                            // HACK LANG - rust's shit replacement for a do-while loop
                            while {
                                self.find_next('*');
                                !self.match_char('/')
                            } {}
                        } else {
                            token.variant = Lexeme::Divide;
                        }
                    },
                    // TODO - raw string literals
                    '\"' => {
                        token.variant = Lexeme::String(self.match_string('\"'));
                    },
                    '\'' => {
                        token.variant = Lexeme::String(self.match_string('\''));
                    },
                    c @ '0'..='9' => {
                        // DESIGN - identifiers can't start with a digit
                        token.variant = Lexeme::Literal(self.match_numeric(c));
                    },
                    _ => {
                    }
                }

                token
            }
        }
    }

    pub fn scan(&mut self) {
        let token = self.advance();
        self.tokens.push(token);
    }

    pub fn print(&self) {
        self.tokens.iter()
            .for_each(|token| println!("{:?}", token));
    }
}
