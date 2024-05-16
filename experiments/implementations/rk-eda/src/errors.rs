use std::error::Error;
use std::fmt;

#[derive(Debug)]
pub enum Errors {
    FailedToParse,
    RowsDifferentLength,
}

impl fmt::Display for Errors {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        match self {
            Self::FailedToParse => write!(f, "Parse error"),
            Self::RowsDifferentLength => {
                write!(f, "All rows must have the same length as the instance size")
            }
        }
    }
}

impl Error for Errors {}
