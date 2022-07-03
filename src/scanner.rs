use crate::token::*;

use std::i64;
use std::iter::Peekable;
use std::str::Chars;

pub struct Scanner<'a> {
    source: Peekable<Chars<'a>>,
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
            source: source.chars().peekable(),
            curr_col: 1,
            curr_row: 1,
        }
    }

    fn carriage_return(&mut self) {
        self.curr_col = 1;
        self.curr_row += 1;
    }

    fn peek(&mut self) -> Option<char> {
        self.source.peek().map(|c| *c)
    }

    fn next(&mut self) -> Option<char> {
        self.curr_col += 1;
        self.source.next()
    }

    fn good(&mut self) -> bool {
        match self.source.peek() {
            None => true,
            Some(_) => false
        }
    }

    fn skip_whitespace(&mut self) {
        while let Some(c) = self.peek() {
            let whitespace = is_whitespace(c as u8);
            match whitespace {
                0x01 => {
                    self.next();
                }
                0x02 => {
                    self.next();
                    self.carriage_return();
                }
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

        if res {
            self.next();
        }
        res
    }

    fn find_next(&mut self, c: char) {
        while let Some(byte) = self.next() {
            if byte == '\n' {
                self.carriage_return();
            }

            if byte == c {
                break;
            }
        }
    }

    fn match_string(&mut self, delimit: char) -> String {
        let mut val = String::new();

        while let Some(byte) = self.next() {
            let mut c = byte;

            if byte == delimit {
                break;
            }

            if byte == '\n' {
                self.carriage_return();
            } else if byte == '\\' {
                c = match self.next() {
                    // TODO ERROR - better error handling here
                    None => {
                        panic!("End of file while parsing string");
                    }
                    Some(escape) => match escape {
                        'n' => {
                            self.carriage_return();
                            '\n'
                        }
                        _ => escape,
                    },
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
                }
                '0'..='9' => {
                    buff.push(c);
                }
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
                }
                '0'..='9' | 'a'..='f' | 'A'..='F' => {
                    buff.push(c);
                }
                _ => {
                    break;
                }
            }
        }

        buff
    }

    fn match_numeric(&mut self, first: char) -> LiteralVariant {
        // floats may not be anything but decimal, does rust even allow it?
        let mut numeric = Numeric::default();
        let mut num: String = first.to_string();

        // TODO - binary, octal, and hex numbers
        /*
        if first == '0' {
            num = match self.peek() {
                None => '0'.to_string(),
                Some(c) => {
                    match c {
                        'b' => {
                            numeric.base = NumericBase::Binary;
                            self.next();
                            self.match_decimal()
                        }
                        // do people actually use octal?
                        'o' => {
                            numeric.base = NumericBase::Octal;
                            self.next();
                            self.match_decimal()
                        }
                        'x' => {
                            numeric.base = NumericBase::Hexadecimal;
                            self.next();
                            self.match_hex()
                        }
                        '0'..='9' | '_' | '.' | 'e' => self.match_decimal(),
                        _ => '0'.to_string(),
                    }
                }
            };
        }
        */

        loop {
            match self.peek() {
                Some(c) => {
                    if is_digit(c) {
                        num.push(c);
                        self.next();
                    } else {
                        break;
                    }
                },
                None => break
            }
        }

        numeric.value = num;

        LiteralVariant::Integer(numeric)
    }

    pub fn advance(&mut self) -> Token {
        self.skip_whitespace();

        let mut token = Token::default();
        let mut location = Location::default();
        location.col = self.curr_col;
        location.row = self.curr_row;
        token.location = location;

        match self.next() {
            None => {}
            Some(byte) => {
                match byte {
                    '(' => {
                        token.variant = Lexeme::LeftParens;
                    }
                    ')' => {
                        token.variant = Lexeme::RightParens;
                    }
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
                    }
                    // TODO - raw string literals
                    '\"' => {
                        token.variant = Lexeme::String(self.match_string('\"'));
                    }
                    '\'' => {
                        token.variant = Lexeme::String(self.match_string('\''));
                    }
                    c @ '0'..='9' => {
                        // DESIGN - identifiers can't start with a digit
                        token.variant = Lexeme::Literal(self.match_numeric(c));
                    }
                    _ => {}
                }
            }
        };

        token
    }
}
