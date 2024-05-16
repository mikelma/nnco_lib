use csv::Writer;
use permu_utils::invert_pop;
use structopt::StructOpt;
extern crate time;
use std::process;
use std::time::Instant;

mod permu_utils;
mod problems;
use problems::ProblemInstance;

mod umda;
use umda::Model;
use umda::ModelType;

mod test; //NOTE: FOR DEBUGGING

mod hungarian;

/// Bijective transformations for permutation based problems
#[derive(Debug, StructOpt)]
struct Cli {
    /// Experiments identificator.
    #[structopt(long = "id", default_value = "0")]
    id: String,

    /// UMDA model to use. 0: Marginal, 1: Vj.
    #[structopt(long = "model")]
    model: i32,

    /// Set 1 to evaluate the inverse of the populations.
    #[structopt(long = "invert", default_value = "0")]
    invert: u8,

    /// Problem to optimize. QAP: 0, PFSP: 1.
    #[structopt(long = "problem")]
    problem: i32,

    /// Path of the problem's instance.
    #[structopt(long = "instance")]
    path: String,

    /// Path of the experiment output csv (detailed).
    #[structopt(long = "output", default_value = "None")]
    out_path: String,

    /// Path of the main csv.
    #[structopt(long = "main-out")]
    main_out: String,

    /// Population size. Default is 100n.
    #[structopt(long = "pop-size")]
    pop_size: Option<usize>,

    /// Truncation size (num. selected samples). Default is pop_size/2.
    #[structopt(long = "truncate")]
    trunc_size: Option<usize>,

    /// Number of iterations. Default is (1000n^2)/pop_size.
    #[structopt(long = "max-iters")]
    max_iters: Option<usize>,

    /// Set 1 to verbose data about iterations.
    #[structopt(short = "v", long = "verbose", default_value = "0")]
    verbose: u8,

    /// Set to 1 to save the distribution at every 10 iterations.
    #[structopt(long = "log-distribution")]
    log_distrib: Option<String>,
}

fn main() {
    let args = Cli::from_args();

    // Read instance
    let instance: ProblemInstance = match args.problem {
        0 => match problems::qap::load_instance(&args.path) {
            Ok(ins) => ins,
            Err(e) => {
                eprintln!("Error loading the problem's instance: {}", e);
                process::exit(1);
            }
        },
        1 => match problems::pfsp::load_instance(&args.path) {
            Ok(ins) => ins,
            Err(e) => {
                eprintln!("Error loading the problem's instance: {}", e);
                process::exit(1);
            }
        },
        _ => {
            eprintln!("Please select a valid problem.");
            process::exit(1);
        }
    };

    // Get instance size
    let size = match instance {
        ProblemInstance::Instanceqap(s, _, _) => s,
        ProblemInstance::Instancepfsp(s, _) => s,
    };

    // ------------- HYPERPARAMETERS -------------
    let pop_size = match args.pop_size {
        Some(v) => v,
        None => size * 100,
    };
    let iterations = match args.max_iters {
        Some(v) => v,
        None => (1000*size.pow(2)) / pop_size,
    };
    let n_surv = match args.trunc_size {
        Some(v) => v,
        None => pop_size / 2,
    };
    let n_samples = pop_size;
    // -------------------------------------------

    // ---------- INITIALIZATIONS --------------
    let mut model = match args.model {
        0 => Model::new(size, ModelType::Marginal),
        1 => Model::new(size, ModelType::Vj),
        _ => {
            eprintln!("Not valid model was given: {}", args.model);
            process::exit(1);
        }
    };

    let mut pop = permu_utils::random_population(&size, &pop_size); // Generate a initial random population
    let mut pop_f: Vec<usize> = vec![0; pop_size]; // Initialize population's fiteness list

    // Initialize survivors
    let mut surv = match model.model_type {
        ModelType::Marginal => permu_utils::zeros_u8(n_surv, size),
        ModelType::Vj => permu_utils::zeros_u8(n_surv, size - 1),
    };

    let mut samples = permu_utils::zeros_u8(n_samples, size); // Initialize Samples
    let mut samples_f: Vec<usize> = vec![0; n_samples]; // Samples fitness list

    let mut b_min = vec![]; // Initialize population's minimum fitness buffer
    let mut b_mean = vec![]; // Initialize population's mean fitness buffer
    let mut b_smean = vec![]; // Initialize sampled solutions' mean fitness buffer
    let mut b_smin = vec![]; // Initialize sampled solutions' min fitness buffer
    let mut b_smax = vec![]; // Initialize sampled solutions' max fitness buffer
    let mut b_smedian = vec![]; // Initialize sampled solutions' median fitness buffer
    let mut b_added = vec![]; // Buffer for how many new solutions were added in each iteration to the population
    let mut mode_p = vec![]; // Mode probability to be sampled
    let mut sampledf_std = vec![]; // Sampled solution's fitness standard deviation

    // Select to eval permutations or their inverse
    let eval_inverse = match args.invert {
        0 => false,
        1 => true,
        _ => {
            eprintln!("Please select a valid option for invert: 1 to enable, 0 to disable.");
            process::exit(1);
        }
    };

    let verbose = match args.verbose {
        0 => false,
        1 => true,
        _ => {
            eprintln!("Please select a valid option for verbose: 1 to enable, 0 to disable.");
            process::exit(1);
        }
    };
    // -------------------------------------------

    let mut distr_log: Vec<(usize, Vec<usize>, usize)> = vec![];
    if args.log_distrib.is_some() {
        println!("[NOTE] Logging the probability distribution every 10 iterations is enabled.");
    }

    let elapsed = Instant::now();
    for iter in 0..iterations {
        // Check if there are only permutations in the population
        // NOTE: ONLY FOR DEBUGGING
        debug_assert!(
            permu_utils::all_permus(&pop, false),
            "All solutions of the population are not permutations."
        );
        if iter != 0 {
            debug_assert!(
                permu_utils::all_permus(&samples, false),
                "All solutions of the population are not permutations."
            );
        }

        if eval_inverse {
            permu_utils::invert_pop(&mut pop);
        }

        // Evaluate population
        for i in 0..pop.len() {
            if args.problem == 0 {
                pop_f[i] = problems::qap::evaluate(&pop[i], &instance);
            } else if args.problem == 1 {
                pop_f[i] = problems::pfsp::evaluate(&pop[i], &instance);
            }
        }

        // Recover original population if reversed
        if eval_inverse {
            permu_utils::invert_pop(&mut pop);
        }

        // Log data, population's min and max fitnesses
        let (mean, min, _, _) = permu_utils::vec_metrics(&pop_f);
        b_min.push(min);
        b_mean.push(mean);

        // Print some info about the current generation
        if verbose {
            println!("iter {}/{} mean: {}, min: {}", iter, iterations, mean, min);
        }

        // Generate population's ranking
        let mut pop_ranking = permu_utils::argsort(&pop_f);

        // Select the best solutions and copy them to surv matrix
        // NOTE: If required a transformation is done to the survivors
        for i in 0..n_surv {
            match model.model_type {
                ModelType::Marginal => surv[i] = pop[pop_ranking[i]].clone(),
                ModelType::Vj => permu_utils::permu2vj(&pop[pop_ranking[i]], &mut surv[i]),
            }
        }

        // Learn distribution based on surv solutions
        model.learn_distribution(&surv);

        // Sample new solutions
        match model.sample(&mut samples) {
            Ok(_) => (),
            Err(e) => {
                eprintln!("Error during sampling new solutions {}", e);
                process::exit(1);
            }
        }

        // Reverse samples if needed
        if eval_inverse {
            permu_utils::invert_pop(&mut samples);
        }

        // Evaluate samples
        for i in 0..n_samples {
            if args.problem == 0 {
                samples_f[i] = problems::qap::evaluate(&samples[i], &instance);
            } else if args.problem == 1 {
                samples_f[i] = problems::pfsp::evaluate(&samples[i], &instance);
            }
        }

        // Recover original samples if they were reversed inthe evaluation
        if eval_inverse {
            permu_utils::invert_pop(&mut samples);
        }

        // If experiment's log is activated save the needed data about the iteration
        if args.out_path != "None" {
            // Log the probability of the mode to be sampled
            match model.mode() {
                Ok(mode) => mode_p.push(mode.1),
                Err(e) => {
                    println!("{}", e);
                    process::exit(1);
                }
            }
            // Store some data about the sampled solutions
            let (smean, smin, smedian, smax) = permu_utils::vec_metrics(&samples_f);
            b_smean.push(smean);
            b_smin.push(smin);
            b_smedian.push(smedian);
            b_smax.push(smax);

            sampledf_std.push(match permu_utils::std(&samples_f) {
                Ok(s) => s,
                Err(e) => {
                    println!("Error: {}", e);
                    process::exit(1);
                }
            });
        }

        // If a sampled solution exists in pop, discard it
        let mut discard: Vec<usize> = vec![];

        // Discard repeated solutions in the samples matrix
        permu_utils::discard_repeated_same_pop(&samples, &samples_f, &mut discard);

        // Discard samples that already exist in the population
        permu_utils::discard_repeated_different_pop(
            &samples,
            &samples_f,
            &pop,
            &pop_f,
            &mut discard,
        );

        // Generate samples ranking
        let samples_ranking = permu_utils::argsort(&samples_f);
        pop_ranking.reverse(); // Worst solutions first

        let mut i_s = 0; // Samples' iteratior
        let mut i_p = 0; // Popuation's iterator
        let mut n_added = 0; // Counter for how many solutions were added to the population
        let mut stop = false;

        while i_s < n_samples && i_p < pop_size && !stop {
            let s_indx = samples_ranking[i_s]; // Sample's index
            let p_indx = pop_ranking[i_p]; // Population solution's index

            let s_f = samples_f[s_indx]; // Sample's fitness
            let p_f = pop_f[p_indx]; // Population solution's fitness

            if !permu_utils::is_in(&s_indx, &discard) {
                // If the sample is not discarded
                if s_f < p_f {
                    // If the fitness of the sample is better that population's solution's
                    pop[p_indx] = samples[s_indx].clone(); // Clone the sample to the population matrix
                    n_added += 1; // Update new solutions counter
                } else {
                    stop = true; // Else, samples are worst than solutions from the population, so stop
                }
                i_p += 1; // Pass to the next solution of the population
            }
            i_s += 1; // Pass to the next sample
        }

        // If experiment's log is activated log the data
        if args.out_path != "None" {
            b_added.push(n_added);
        }

        // Save the new prob. distribution of required
        if args.log_distrib.is_some() {
            // let best_f = if distr_log.is_empty() || distr_log.last().unwrap().2 < min { min } else { distr_log.last().unwrap().2 };
            for row in model.distribution.as_ref().unwrap() {
                distr_log.push((iter, row.clone(), min));
            }
        }
    }

    // ---- DATA LOGS ---- //

    let elapsed_time = elapsed.elapsed().as_secs().to_string();


    // Log prob. distrib evolution if required
    if let Some(path) = args.log_distrib {
        let mut log_file = Writer::from_path(path).expect("Error while creating the divergence csv file");

        log_file.write_record(&["iteration", "distribution", "best fitness"]).unwrap();
        for (i, v, b) in distr_log {
            log_file.write_record(&[i.to_string(), format!("{:?}", v), b.to_string()]).unwrap();
        }
        log_file.flush().unwrap();
    }

    // Format current datetime
    let now = time::now();
    let date = format!(
        "{}-{}-{} {}:{}",
        now.tm_year + 1900,
        now.tm_mon + 1,
        now.tm_mday,
        now.tm_hour,
        now.tm_min
    );

    // Define used instance name
    let split = args.path.split("/");
    let vec: Vec<&str> = split.collect();
    let instance_name = vec[vec.len() - 1];

    // Define problems name
    let problem_name = instance.name();

    // Define search space name
    let mut model_name = match model.model_type {
        ModelType::Marginal => String::from("marginal"),
        ModelType::Vj => String::from("vj"),
    };

    if args.invert == 1 {
        model_name.push_str("-inv");
    }

    // Open main log
    let mut main_log =
        Writer::from_path(args.main_out).expect("Error while creating the main csv file");

    main_log
        .write_record(&[
            "id",
            "date",
            "problem",
            "instance",
            "iterations",
            "model",
            "pop size",
            "trunc size",
            "min",
            "elapsed time in secs",
        ])
        .expect("Error writing the header to the main log"); // Write header
                                                             // Write data to experiment log
    main_log
        .write_record(&[
            &args.id,
            &date,
            problem_name,
            instance_name,
            &iterations.to_string(),
            &model_name,
            &pop_size.to_string(),
            &(n_surv as f32 / pop_size as f32).to_string(),
            &(b_min[b_min.len() - 1].to_string()),
            &elapsed_time,
        ])
        .expect("Failed writing data to the main csv log");

    main_log.flush().expect("Failed to flush main log");

    // If experiment's log was activated, write data to it
    if args.out_path != "None" {
        let mut wtr =
            Writer::from_path(args.out_path).expect("Error while creating the experiment's log");

        // Write header
        wtr.write_record(&[
            "iteration",
            "problem",
            "instance",
            "model",
            "mean",
            "min",
            "sampled mean",
            "sampled min",
            "sampled median",
            "sampled max",
            "added solutions",
            "mode p",
            "sampled fitness std",
        ])
        .expect("Failed writing the header of the experiments csv log"); // Write header

        // Write data to experiment's log
        for i in 0..iterations {
            let iter = i.to_string();
            let mean = b_mean[i].to_string();
            let min = b_min[i].to_string();
            let smean = b_smean[i].to_string();
            let smin = b_smin[i].to_string();
            let smedian = b_smedian[i].to_string();
            let smax = b_smax[i].to_string();
            let added = b_added[i].to_string();
            let pmode = mode_p[i].to_string();
            let sf_std = sampledf_std[i].to_string();

            wtr.write_record(&[
                &iter,
                problem_name,
                &instance_name,
                &model_name,
                &mean,
                &min,
                &smean,
                &smin,
                &smedian,
                &smax,
                &added,
                &pmode,
                &sf_std,
            ])
            .expect("Failed to write a row in the experiment's log");
        }
        wtr.flush().expect("Failed to flush experiment's log");
    }
}
