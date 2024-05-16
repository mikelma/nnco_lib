#[derive(Debug)]
pub enum ProblemInstance {
    Instanceqap(usize, Vec<Vec<usize>>, Vec<Vec<usize>>),
    Instancepfsp(usize, Vec<Vec<usize>>),
}

impl ProblemInstance {
    pub fn name(&self) -> &str {
        match *self {
            ProblemInstance::Instanceqap(_, _, _) => "QAP",
            ProblemInstance::Instancepfsp(_, _) => "PFSP",
        } 
    }  
}

pub mod qap {

    use crate::problems::ProblemInstance;

    pub fn load_instance(path: &str) -> Result<ProblemInstance, std::io::Error>  {

        use std::fs::File;
        use std::io::{BufRead, BufReader};

        let filename = path; 
        // Open the file in read-only mode (ignoring errors).
        let file = File::open(filename)?;
        /*
        let file = match File::open(filename) {
            Ok(f) => f,
            Err(e) => return Err,
        };
        */

        let reader = BufReader::new(file);
        
        let mut size: usize = 0; // Init instance size variable 
        let mut dist = vec![];   // Init distances matrix
        let mut flow = vec![];   // Init flow matrix

        // Read the file line by line using the lines() iterator from std::io::BufRead.
        for (index, line) in reader.lines().enumerate() {
            
            let line = line.unwrap(); // Ignore errors.
             
            let mut row: Vec<usize> = Vec::new(); // Init row vector
                
            // Define the size of the problem
            if index == 0 {
                size = line.trim().parse().unwrap();

            // Build distances matrix
            } else if index <= size && size != 0 {

                let data = line.split(" ");
                
                for d in data {
                    match d.parse::<usize>() {
                        Ok(a) => row.push(a),
                        Err(_e) => (),
                    }
                }
                dist.push(row); 

            // Build flow matrix
            } else {
                let data = line.split(" ");
                
                for d in data {
                    match d.parse::<usize>() {
                        Ok(a) => row.push(a),
                        Err(_e) => (),
                    }
                }
                flow.push(row); 
            }
        }
        // Return instance size, distance and flow matrixes
        // (size, dist, flow)
        Ok(ProblemInstance::Instanceqap(size, dist, flow))
    }
    
    pub fn evaluate(permu: &Vec<u8>, 
                    instance: &ProblemInstance) -> usize {
        
        let (size, dist, flow) = match instance {
            ProblemInstance::Instanceqap(size, dist, flow) => (size, dist, flow),
            _ => panic!("The given instance, does not match with a qap instance type. Can not evaluate."),
        }; 

        let mut fitness = 0; 

        *size as u8;

        for i in 0..*size {
            for j in 0..*size {

                let fact_a = permu[i] as usize;
                let fact_b = permu[j] as usize;

                let dist_ab = dist[i][j];
                let flow_ab = flow[fact_a][fact_b];

                fitness += dist_ab*flow_ab;
            }
        }
        fitness
    }
}

pub mod pfsp {

    use crate::problems::ProblemInstance;

    // pub fn load_instance(path: &str) -> (usize, Vec<Vec<usize>>) {
    pub fn load_instance(path: &str) -> Result<ProblemInstance, std::io::Error> {

        use std::fs::File;
        use std::io::{BufRead, BufReader};

        let filename = path; 
        // Open the file in read-only mode (ignoring errors).
        let file = File::open(filename)?;
        let reader = BufReader::new(file);
        
        let mut instance = vec![];   // Init instance matrix

        // Read the file line by line using the lines() iterator from std::io::BufRead.
        for (index, line) in reader.lines().enumerate() {
            
            let line = line.unwrap(); // Ignore errors.
             
            let mut row: Vec<usize> = Vec::new(); // Init row vector
                
            // Build distances matrix
            if index > 2 {

                let data = line.split(" ");
                
                for d in data {
                    match d.parse::<usize>() {
                        Ok(a) => row.push(a),
                        Err(_e) => (),
                    }
                }
                instance.push(row); 
            }
        }
        let size = instance[0].len();
        // Return instance size, distance and flow matrixes
        // (size, instance)
        Ok(ProblemInstance::Instancepfsp(size, instance))
    }

    // pub fn evaluate(permu: &mut Vec<u8>, instance: &Vec<Vec<usize>>) -> usize {
    pub fn evaluate(permu: &Vec<u8>, p_instance: &ProblemInstance) -> usize {

        use std::cmp::max; // NOTE: Remove use from here

        let instance = match p_instance {
            ProblemInstance::Instancepfsp(_, ins) => ins,
            _ => panic!("The given instance, does not match with a qap instance type. Can not evaluate."),
        }; 

        let n_machines = instance.len();
        let mut tft = 0;
        let mut b = vec![0;n_machines];  

        for (job_i, job) in permu.iter().enumerate() {
            let mut pt = 0;
            for machine in 0..n_machines {

                job_i as usize;
                let job = *job as usize;

                if job_i == 0 && machine == 0 {
                    pt = instance[machine][job];
                }
                else if job_i > 0 && machine == 0 {
                    pt = b[machine] + instance[machine][job];
                }
                else if job_i == 0 && machine > 0 {
                    pt = b[machine-1] + instance[machine][job];
                }
                else if job_i > 0 && machine > 0 {
                    pt = max(b[machine-1], b[machine]) + instance[machine][job];
                }

                b[machine] = pt;
            }
            tft += pt;
        }
        tft
    }
}
