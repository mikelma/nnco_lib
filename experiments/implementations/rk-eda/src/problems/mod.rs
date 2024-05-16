use std::error::Error;
use std::fs::File;
use std::io::{BufRead, BufReader};

use crate::Population;

pub mod lop;
pub mod pfsp;

pub enum ProblemType {
    Lop,
    Pfsp,
}

pub trait Problem {
    fn size(&self) -> usize;
    fn from_file(path: &str) -> Result<Box<dyn Problem>, Box<dyn Error>>
    where
        Self: Sized;
    fn eval_population(&self, pop: &Population) -> Vec<usize>;
}

/// Utility function to convert a buffer into a matrix of the specified shape.
fn lines2matrix(
    buffer: &mut BufReader<File>,
    n_lines: usize,
    n_elems: usize,
) -> Result<Vec<Vec<usize>>, &str> {
    let mut matrix = vec![Vec::with_capacity(n_elems); n_lines];
    for m_row in matrix.iter_mut() {
        // read the line and split in withespaces
        let mut line = String::new();
        buffer.read_line(&mut line).unwrap();
        let line = line.split_whitespace();
        // parse all numbers from str to usize
        let mut count = 0;
        for str_num in line {
            m_row.push(match str_num.trim().parse() {
                Ok(n) => n,
                Err(_) => return Err("Parsing error"),
            });
            count += 1;
        }
        // check if line length is ok
        if count != n_elems {
            return Err("All rows must have the same length as the instance size");
        }
    }
    Ok(matrix)
}
