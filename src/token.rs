#[derive(Clone, Debug, PartialEq)]
pub enum Lexeme {
    Eof,
    // single char tokens
    LeftParens,   // (
    RightParens,  // )
    LeftBrace,    // {
    RightBrace,   // }
    Comma,        // ,
    Dot,          // .
    SemiColon,    // ;
    // one or two char token,
    Minus,        // -
    MinusEqual,   // -=
    Plus,         // +
    PlusEqual,    // +=
    Slash,        // /
    SlashEqual,   // /=
    Star,         // *
    StarEqual,    // *=
    Bang,         // !
    BangEqual,    // !=
    Assignment,   // =
    Equality,     // ==
    Greater,      // >
    GreaterEqual, // >=
    Less,         // <
    LessEqual,    // <=
    // literals
    Identifier,
    String,
    Number,
    // keywords
    And,
    Defer,
    Else,
    False,
    For,
    Function,
    If,
    Or,
    Return,
    Struct,
    True,
    Var,
    While,
    // misc
    Error(std::string::String),
    Comment,
}

#[derive(Clone, Debug)]
pub struct Token {
    pub variant: Lexeme,
    // index into the source file
    pub idx: u32,
    pub col: u32,
    pub line: u32,
    pub len: u32,
}
