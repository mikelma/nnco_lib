use rand::prelude::*;

use std::cmp::Ordering;

use super::Population;
use crate::Solution;

pub fn random_population(sol_size: usize, num_sol: usize) -> Population {
    let mut rng = rand::thread_rng();
    (0..num_sol)
        .map(|_| {
            let mut sol = (0..sol_size).collect::<Solution>();
            sol.shuffle(&mut rng);
            sol
        })
        .collect()
}

pub fn argsort<T: Ord>(data: &[T]) -> Vec<usize> {
    let mut indices = (0..data.len()).collect::<Vec<_>>();
    indices.sort_by_key(|&i| &data[i]);
    indices
}

pub fn argsort_f64(arr: &Vec<f64>) -> Vec<usize> {
    let mut out = (0..arr.len()).collect::<Vec<usize>>();
    out.sort_by(|&a_idx, &b_idx| {
        let a = arr[a_idx];
        let b = arr[b_idx];
        match (a.is_nan(), b.is_nan()) {
            (true, true) => Ordering::Equal,
            (true, false) => Ordering::Greater,
            (false, true) => Ordering::Less,
            (false, false) => a.partial_cmp(&b).unwrap(),
        }
    });
    out
}
