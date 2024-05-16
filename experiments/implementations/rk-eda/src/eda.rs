use super::{Objective, Population};
use crate::utils;
use rand_distr::{Distribution, Normal};

pub trait Eda {
    fn repopulate(&mut self, pop: &mut Population, fitness: &[usize]);
    fn update_std(&mut self, _new_std: f64) {}
    fn get_distrib(&self) -> &[f64];
}

pub struct RkEda {
    rks: Vec<Vec<f64>>,
    means: Vec<f64>,
    std: f64,
    /// Number of solutions to select to learn the distrib. from
    n_select: usize,
    obj: Objective,
}

impl RkEda {
    pub fn new(
        sol_size: usize,
        pop_size: usize,
        std_start: f64,
        n_select: usize,
        obj: Objective,
    ) -> Self {
        let rks = vec![vec![0.0; sol_size]; pop_size];
        let means = vec![0.0; sol_size];
        RkEda {
            rks,
            means,
            std: std_start,
            n_select,
            obj,
        }
    }
}

impl Eda for RkEda {
    fn get_distrib(&self) -> &[f64] {
        &self.means
    }

    fn repopulate(&mut self, pop: &mut Population, fitness: &[usize]) {
        let m = pop.len();
        let n = pop[0].len();

        let mut sol_raking = utils::argsort(&fitness);

        if self.obj == Objective::Maximize {
            sol_raking.reverse();
        }

        // indices of the first `n_select` best solutions
        let selected = &sol_raking[0..self.n_select];

        // rescale rankings to [0, 1] values and compute means
        let norm_v = (n - 1) as f64;
        for &i in selected {
            for j in 0..n {
                let normalized = (pop[i][j] as f64) / norm_v;
                self.rks[i][j] = normalized;
                self.means[j] += normalized / (m as f64);
            }
        }

        let normals: Vec<Normal<f64>> = (0..n)
            .map(|i| Normal::new(self.means[i], self.std).unwrap())
            .collect();

        let mut rng = rand::thread_rng();
        for i in 0..m {
            let rk: Vec<f64> = normals
                .iter()
                .map(|normal| normal.sample(&mut rng))
                .collect(); // generate the RK
            let sol = utils::argsort_f64(&rk); // ranking from the RK
            for j in 0..n {
                // copy solution to the population
                pop[i][j] = sol[j];
            }
        }
    }

    fn update_std(&mut self, new_std: f64) {
        self.std = new_std;
    }
}
