use std::error::Error;
use std::fs::File;
use std::io::{BufRead, BufReader};

use super::Problem;
use crate::Population;

pub struct Lop {
    matrix: Vec<Vec<usize>>,
    size: usize,
}

impl Problem for Lop {
    fn size(&self) -> usize {
        self.size
    }

    fn from_file(path: &str) -> Result<Box<dyn Problem>, Box<dyn Error>>
    where
        Self: Sized,
    {
        let file = File::open(path)?;
        let mut reader = BufReader::new(file);

        // read and parse the line containing the size of the matrix
        let mut size_str = String::new();
        let _n = reader.read_line(&mut size_str);
        let size: usize = size_str.trim().parse()?;

        // read the instance matrix
        let matrix = super::lines2matrix(&mut reader, size, size).unwrap();

        Ok(Box::new(Lop { size, matrix }))
    }

    fn eval_population(&self, solutions: &Population) -> Vec<usize> {
        let mut fitness_vec = Vec::with_capacity(solutions.len());
        let n = self.matrix.len();

        solutions.iter().for_each(|solution| {
            fitness_vec.push(
                solution
                    .iter()
                    .enumerate()
                    .take(n - 1)
                    .map(|(i, sig_i)| {
                        solution
                            .iter()
                            .skip(i + 1)
                            .map(|sig_j| self.matrix[*sig_i][*sig_j])
                            .sum::<usize>()
                    })
                    .sum::<usize>(),
            );
        });
        fitness_vec
    }
}
