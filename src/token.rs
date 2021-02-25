use std::fmt;

#[derive(Debug)]
pub enum LiteralVariant {
    Integer(i64),
    Floating(f64),
}

// TODO - rustc lexes whitespace, probably for better error reporting
#[derive(Debug)]
pub enum Lexeme {
    Eof,
    String(String),
    Literal(LiteralVariant),
    Identifier,
    LeftParens,
    RightParens,
    Assign,
    Equality,
    Comment,
    Add,
    Subtract,
    Multiply,
    Divide,

    Function,
    If,
    Else,
    For,
    While,
    Return,
    Var,
    True,
    False,
}

const MAX_TOKEN_ID_SIZE: usize = 256;

pub struct Identifier {
    length: u32,
    name: [u8; MAX_TOKEN_ID_SIZE],
}

impl Identifier {
    pub fn new(name: &str) -> Identifier {
        assert!(name.len() <= MAX_TOKEN_ID_SIZE);

        let mut buf: [u8; MAX_TOKEN_ID_SIZE] = [0; MAX_TOKEN_ID_SIZE];
        let mut idx = 0;
        for c in name.as_bytes() {
            buf[idx] = *c;
            idx += 1;
        }

        Identifier {
            length: name.len() as u32,
            name: buf,
        }
    }
}

impl fmt::Debug for Identifier {
    fn fmt(&self, f: &mut fmt::Formatter) -> fmt::Result {
        let name: Vec<u8> = self.name.iter()
            .take(self.length as usize)
            .map(|c| *c)
            .collect();

        // look, we're going from text to &str back to text, it's fine
        unsafe {
            let name: &str = std::str::from_utf8_unchecked(&name);
            write!(f, "{}", name)
        }
    }
}

#[derive(Debug)]
pub struct Token {
    pub variant: Lexeme,
    pub row: u32,
    pub col: u32,
    pub identifier: Identifier,
}

impl Default for Token {
    fn default() -> Token {
        Token {
            variant: Lexeme::Eof,
            row: 0,
            col: 0,
            identifier: Identifier::new(""),
        }
    }
}
