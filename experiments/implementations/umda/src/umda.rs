#[derive(Debug)]
pub enum ModelType {
    Marginal,
    Vj,
}

#[derive(Debug)]
pub struct Model {
    pub distribution : Option<Vec<Vec<usize>>>,
    size : usize,
    pub model_type : ModelType,
}

use crate::permu_utils;

impl Model {

    pub fn new(size: usize, model_type: ModelType) -> Model {
        Model {
            size : size,
            model_type : model_type,
            distribution : None,
        }
    }

    pub fn clone_distribution(&self) -> Result<Vec<Vec<usize>>, &'static str> {
        match &self.distribution {
            Some(d) => Ok(d.clone()),
            None => Err("Can't clone an empty distibution"),
        }
    }

    pub fn learn_distribution(&mut self, pop: &Vec<Vec<u8>>) {
        let n = self.size;          // Number of possible values
        let m = pop[0].len();   // Number of positions

        //NOTE: To Optimize. This initialization should be avoided if distribution is Some, and
        //shold be restored to zeros insted of reinitialized.
        let mut distr: Vec<Vec<usize>> = vec![vec![0; n]; m]; // Init distribution matrix

        for i in 0..pop.len(){
            for j in 0..pop[0].len() {
                let a = pop[i][j] as usize;
                distr[j][a] += 1;
            }
        }
        self.distribution = Some(distr);
    }

    pub fn show(&self) {
        println!("Size: {}", self.size);
        println!("Model type: {:?}", self.model_type);
        print!("Distribution:");
        match &self.distribution {
            Some(d) => {
                print!("\n");
                for row in d {
                    println!("{:?}", row);
                }
                println!("shape: {},{}", d.len(), d[0].len())
            },
            None => println!("None"),
        }
    }

    pub fn laplace(&mut self, reverse: bool) -> Result<(), &'static str> {
        let mut d = match &mut self.distribution {
            Some(x) => x,
            None => return Err("Can't apply laplace to an empty distribution"),
        };

        match self.model_type {
            ModelType::Marginal => marginal::laplace(&mut d, reverse),
            ModelType::Vj => vj::laplace_diagonal(&mut d, reverse),
        }
        Ok(())
    }

    pub fn sample<'a>(&mut self, samples: &'a mut Vec<Vec<u8>>) -> Result<(), &'static str> {
        self.laplace(false)?; // Apply laplace

        let distr = match &self.distribution {
            Some(x) => x,
            None => return Err("Can't sample from an empty distribution."),
        };

        let sampler = match self.model_type {
            ModelType::Marginal => marginal::ad_hoc_random_laplace,
            ModelType::Vj => vj::no_restriction_laplace,
        };

        for i in 0..samples.len() {
            samples[i] = sampler(distr);  // Sample
        }

        self.laplace(true)?; // Remove laplace

        Ok(())
    }

    pub fn mode(&mut self) -> Result<(Vec<u8>, f64), &'static str> {
        self.laplace(false)?; // Apply laplace

        let distr = match &self.distribution {
            Some(x) => x,
            None => return Err("Can't sample from an empty distribution."),
        };

        let result = match self.model_type {
            ModelType::Marginal => marginal::mode(distr),
            ModelType::Vj => vj::mode(distr),
        };

        let probability = permu_utils::probability(distr, &result);

        self.laplace(true)?; // Remove laplace

        Ok((result, probability))
    }
}

mod marginal {

    use rand::Rng;
    use crate::permu_utils;
    use crate::hungarian;
    use hungarian::W;

    pub fn laplace<'a>(m: &'a mut Vec<Vec<usize>>,
                       reverse: bool) {
        if !reverse {
            for i in 0..m.len() {
                for j in 0..m[i].len(){
                    m[i][j] += 1;
                }
            }
        } else {
            for i in 0..m.len() {
                for j in 0..m[i].len(){
                    m[i][j] -= 1;
                }
            }
        }
    }

    pub fn ad_hoc_random_laplace(distr: &Vec<Vec<usize>>) -> Vec<u8> {

        let size = distr.len();
        let mut sample: Vec<u8> = vec![0;size];
        let mut used_indx: Vec<u8> = vec![];

        let order = permu_utils::random_permutation(&size);
        for j in order {

            // Calculate max sum
            let mut s_max = 0;
            for i in 0..size {
                if !permu_utils::is_in(&(i as u8), &used_indx) {
                    s_max += distr[j as usize][i];
                }
            }
            let rand: f64 = match s_max {
                0 => 0.0,
                _ => rand::thread_rng().gen_range(0.0, s_max as f64),
            };

            let mut s = 0;
            let mut i = 0;

            while (s as f64) < rand {
                if !permu_utils::is_in(&(i as u8), &used_indx) {
                    s += distr[j as usize][i];
                }
                if (s as f64) < rand {
                    i += 1;
                }
            }
            sample[j as usize] = i as u8;
            used_indx.push(i as u8);
        }
        sample
    }

    pub fn mode(distribution: &Vec<Vec<usize>>) -> Vec<u8> {
        let weights = W::from(distribution);
        weights.hungarian()
    }

}

mod vj {

    use rand::Rng;
    use crate::permu_utils;

    pub fn laplace_diagonal<'a>(m: &'a mut Vec<Vec<usize>>,
                                reverse: bool) {

        let mut row_max = m[0].len();

        if !reverse {
            for i in 0..m.len() {
                for j in 0..row_max {
                    m[i][j] += 1;
                }
                row_max -= 1;
            }

        } else {
            for i in 0..m.len() {
                for j in 0..row_max {
                    m[i][j] -= 1;
                }
                row_max -= 1;
            }
        }
    }

    //NOTE: to optimize
    //NOTE: Only to use with Vj distributions, and returns permutations
    pub fn no_restriction_laplace(distr: &Vec<Vec<usize>>) -> Vec<u8> {

        let mut sample: Vec<u8> = vec![];
        let mut permu: Vec<u8> = vec![0;distr.len()+1];

        for j in 0..distr.len() {

            // The only difference of this function and no_restriction is
            // that the max sum (s_max) is calculed each loop
            let mut s_max = 0;
            for a in 0..distr[j].len() {
                s_max += distr[j][a];
            }

            let rand = rand::thread_rng().gen_range(0.0, s_max as f64);
            let mut i = 0;
            let mut s = 0;

            while (s as f64) < rand {

                s += distr[j][i];

                if (s as f64) < rand {
                    i += 1;
                }
            }
            sample.push(i as u8);
        }
        permu_utils::vj2permu(&sample, &mut permu);
        permu
    }

    pub fn mode(distribution: &Vec<Vec<usize>>) -> Vec<u8> {
        let mut result: Vec<u8> = vec![];

        for pos in 0..distribution.len() {
            result.push(permu_utils::argmax(&distribution[pos]) as u8);
        }
        result
    }
}


#[cfg(test)]
mod test {

    use crate::umda;
    use crate::permu_utils;
    use umda::ModelType;
    use umda::Model;

    #[test]
    fn marginal_init() {
        let model = Model::new(20, ModelType::Marginal);

        match model.model_type {
            ModelType::Marginal => (),
            _ => panic!("Error"),
        }
    }

    #[test]
    fn marginal_disribution() {
        let size = 3;
        let mut model = Model::new(size, ModelType::Marginal);

        let pop: Vec<Vec<u8>> = vec![vec![2,1,0], vec![2,1,0], vec![1,2,0]];

        model.learn_distribution(&pop);

        let d = match model.clone_distribution() {
            Ok(x) => x,
            Err(e) => panic!(e),
        };
        let ok = vec![vec![0,1,2],vec![0,2,1], vec![3,0,0]];
        assert_eq!(ok, d, "Distribution calculation error");

        // Test sampling
        let mut samples = permu_utils::zeros_u8(100, size);
        let zeros = samples.clone();

        model.sample(&mut samples)
            .unwrap();

        let d2 = match model.clone_distribution() {
            Ok(x) => x,
            Err(e) => panic!(e),
        };

        assert_ne!(samples, zeros, "Sampling not working, returning empty solutions");
        assert_eq!(d, d2, "This distributions must be equal");
        permu_utils::all_permus(&samples, true);

        // Apply laplace
        model.laplace(false)
            .unwrap();

        let d3 = match model.clone_distribution() {
            Ok(x) => x,
            Err(e) => panic!(e),
        };
        let ok_laplace = vec![vec![1,2,3],vec![1,3,2], vec![4,1,1]];
        assert_eq!(ok_laplace, d3, "Failed to apply laplace");

        // Remove laplace
        model.laplace(true)
            .unwrap();

        let d4 = match model.clone_distribution() {
            Ok(x) => x,
            Err(e) => panic!(e),
        };
        assert_eq!(ok, d4, "Failed to remove laplace");
    }

    #[test]
    fn vj_in() {
        let mut model = Model::new(20, ModelType::Vj);

        match model.model_type {
            ModelType::Vj => (),
            _ => panic!("Error"),
        }
    }

    #[test]
    fn vj_distribution() {

        let size = 4;
        let mut model = Model::new(size, ModelType::Vj);

        let vj: Vec<Vec<u8>> = vec![vec![2,1,0], vec![3,1,0], vec![1,1,0]];

        model.learn_distribution(&vj);

        let d = match model.clone_distribution() {
            Ok(x) => x,
            Err(e) => panic!(e),
        };
        let ok = vec![vec![0,1,1,1],vec![0,3,0,0], vec![3,0,0,0]];
        assert_eq!(ok, d, "Distribution calculation error");

        // Test sampling
        let mut samples = permu_utils::zeros_u8(100,size);
        let zeros = samples.clone();

        model.sample(&mut samples)
            .unwrap();

        let d2 = match model.clone_distribution() {
            Ok(x) => x,
            Err(e) => panic!(e),
        };

        assert_ne!(samples, zeros, "Sampling not working, returning empty solutions");
        assert_eq!(d, d2, "This distributions must be equal");
        permu_utils::all_permus(&samples, true);

        // Apply laplace
        model.laplace(false)
            .unwrap();

        let d3 = match model.clone_distribution() {
            Ok(x) => x,
            Err(e) => panic!(e),
        };
        let ok_laplace = vec![vec![1,2,2,2],vec![1,4,1,0], vec![4,1,0,0]];
        assert_eq!(ok_laplace, d3, "Failed to apply laplace");

        // Remove laplace
        model.laplace(true)
            .unwrap();

        let d4 = match model.clone_distribution() {
            Ok(x) => x,
            Err(e) => panic!(e),
        };
        assert_eq!(ok, d4, "Failed to remove laplace");
    }

    #[test]
    #[should_panic]
    fn sample_from_empty_distribution() {
        let mut model = Model::new(10, ModelType::Marginal);
        let mut samples = vec![vec![0;10];100];

        match model.sample(&mut samples) {
            Ok(_) => (),
            Err(e) => panic!("{}", e),
        }
    }
}
