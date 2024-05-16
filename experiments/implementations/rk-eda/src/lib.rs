pub mod eda;
pub mod errors;
pub mod problems;
pub mod utils;

pub use eda::{Eda, RkEda};
pub use errors::Errors;
pub use problems::{lop::Lop, pfsp::Pfsp, Problem, ProblemType};

type Solution = Vec<usize>;
type Population = Vec<Solution>;

#[derive(PartialEq, Eq, Debug, Clone, Copy)]
pub enum Objective {
    Maximize,
    Minimize,
}
