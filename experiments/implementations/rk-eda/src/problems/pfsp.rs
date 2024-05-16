use std::cmp::max;
use std::error::Error;
use std::fs::File;
use std::io::{BufRead, BufReader};

use super::Problem;
use crate::Population;

pub struct Pfsp {
    matrix: Vec<Vec<usize>>,
    size: usize,
    n_machines: usize,
}

impl Problem for Pfsp {
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
        let mut info_str = String::new();

        // skip line
        let _n = reader.read_line(&mut info_str);
        info_str.clear();

        let _n = reader.read_line(&mut info_str);
        let mut split = info_str.split_ascii_whitespace();
        let size_str = split.next().unwrap();
        let size: usize = size_str.trim().parse()?;
        let machines_str = split.next().unwrap();
        let n_machines: usize = machines_str.trim().parse()?;

        // skip line
        let _n = reader.read_line(&mut info_str);

        // read the instance matrix
        let matrix = super::lines2matrix(&mut reader, n_machines, size).unwrap();

        Ok(Box::new(Pfsp {
            size,
            n_machines,
            matrix,
        }))
    }

    fn eval_population(&self, solutions: &Population) -> Vec<usize> {
        // check if the solution's length matches with the size of the problem
        // create a vector to hold the fitness values and allocate the needed memory beforehand
        let mut fitness_vec = Vec::with_capacity(solutions.len());
        for solution in solutions {
            let mut tft = 0;
            let mut b = vec![0; self.n_machines];
            for (job_i, job_n) in solution.iter().enumerate() {
                let mut pt = 0;
                for machine in 0..self.n_machines {
                    if job_i == 0 && machine == 0 {
                        pt = self.matrix[machine][*job_n];
                    } else if job_i > 0 && machine == 0 {
                        pt = b[machine] + self.matrix[machine][*job_n];
                    } else if job_i == 0 && machine > 0 {
                        pt = b[machine - 1] + self.matrix[machine][*job_n];
                    } else if job_i > 0 && machine > 0 {
                        pt = max(b[machine - 1], b[machine]) + self.matrix[machine][*job_n];
                    }
                    b[machine] = pt;
                }
                tft += pt;
            }
            fitness_vec.push(tft);
        }
        fitness_vec
    }
}
