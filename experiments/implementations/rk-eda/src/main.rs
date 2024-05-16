use rk_eda::{self, utils, Eda, Lop, Objective, Pfsp, Problem, ProblemType, RkEda};
use std::env;
use std::fs::File;
use std::io::prelude::*;
use std::path::Path;
use std::process::exit;
use std::time::Instant;


const LOG_CONVERGENCE: bool = false;

fn main() {
    //
    // Read CLI arguments
    // ==================
    let mut args = env::args().skip(1);

    let instance_path = match args.next() {
        Some(v) => v,
        None => missing_arg_error("instance path"),
    };

    // guess problem's type from the instance's extension
    let (problem_type, objective) = match Path::new(&instance_path).extension() {
        Some(v) if v == "fsp" => (ProblemType::Pfsp, Objective::Minimize),
        Some(_) => unimplemented!(),
        None => (ProblemType::Lop, Objective::Maximize),
    };

    let res: Result<Box<dyn Problem>, Box<dyn std::error::Error>> = match problem_type {
        ProblemType::Lop => Lop::from_file(&instance_path),
        ProblemType::Pfsp => Pfsp::from_file(&instance_path),
    };
    let problem = match res {
        Ok(p) => p,
        Err(e) => {
            eprintln!("⛔ Error loading instance file: {e}");
            exit(1);
        }
    };

    let pop_size = match args.next() {
        Some(v) => v.parse::<usize>().expect("Failed to parse population size"),
        None => problem.size() * 100,
    };

    let trunc_size = match args.next() {
        Some(v) => v.parse::<usize>().expect("Failed to parse truncation size"),
        None =>  (pop_size as f64 * 0.2) as usize,
    };

    let log_file = args.next();

    let max_iters = 1000 * problem.size().pow(2) / pop_size;
    let mut pop = utils::random_population(problem.size(), pop_size);
    let std_start = 1.0 / (3.14 * (problem.size() as f64).log10());
    let mut eda = RkEda::new(problem.size(), pop_size, std_start, trunc_size, objective);

    //
    // Main loop
    // =========

    let mut biglog = if LOG_CONVERGENCE {
        println!("Logging distribution at every 10 iters. This might add overhead to the algorithm. Go to the code to disable.");
        let mut f = File::create("convergence.csv").unwrap();
        f.write_all(b"iteration,distribution,best fitness\n").unwrap();
        Some(f)
    } else {
        None
    };

    let mut best_fitness = if objective == Objective::Minimize {
        usize::MAX
    } else {
        0
    };
    let mut best_fitness_lst = vec![];
    let t_start = Instant::now();
    for it in 0..max_iters {
        let fitness = problem.eval_population(&pop);
        eda.repopulate(&mut pop, &fitness);

        // update std
        eda.update_std(1.0 - ((it + 1) as f64 / max_iters as f64));

        // ----- logging ----- //
        match objective {
            Objective::Minimize => {
                let m = *fitness.iter().min().unwrap();
                if m < best_fitness {
                    best_fitness = m;
                }
            }
            Objective::Maximize => {
                let m = *fitness.iter().max().unwrap();
                if m > best_fitness {
                    best_fitness = m;
                }
            }
        }
        if log_file.is_some() {
            best_fitness_lst.push(best_fitness);
        }
        // println!("{it}/{max_iters}, best: {best_fitness}");
        // ------------------- //

        if let Some(f) = &mut biglog {
            if it % 10 == 0 && it > 0 {
                f.write_all(format!("{},\"{:?}\",{}\n", it, eda.get_distrib(), best_fitness).as_bytes()).unwrap();
            }
        }
    }
    let elapsed = t_start.elapsed().as_secs();

    //
    // Logging
    // =======
    let instance = Path::new(&instance_path)
        .file_stem()
        .unwrap()
        .to_str()
        .unwrap();
    let header = "algorithm,instance,instance size,pop size,trunc size,max iters,iteration,best fitness,elapsed seconds";
    if let Some(fname) = log_file {
        let mut f = File::create(&fname).unwrap();
        writeln!(f, "{header}").unwrap();
        for (i, v) in best_fitness_lst.iter().enumerate() {
            writeln!(
                f,
                "rk-eda,{instance},{},{pop_size},{trunc_size},{max_iters},{i},{v},{elapsed}",
                problem.size()
            )
            .unwrap();
        }

        println!("🪂 Training curves written to: {fname}");
    } else {
        println!("{header}");
        println!(
            "rk-eda,{instance},{},{pop_size},{trunc_size},{max_iters},{max_iters},{best_fitness},{elapsed}",
            problem.size()
        );
    }
}

fn missing_arg_error(missing_arg: &str) -> ! {
    eprintln!("⚠️ Missing argument: {missing_arg}");
    eprintln!("Usage:\n\trk-eda [INSTANCE_PATH:str] [?POP_SIZE:uint] [?TRUNC_SIZE:uint] [?LOG_FILE]");
    exit(1);
}
