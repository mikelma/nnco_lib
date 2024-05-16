#[cfg(test)]
mod tests {

    use crate::permu_utils;
    use crate::umda;
    // use crate::umda::SamplingFunction;
    use crate::problems;

    // ----- PERMU UTILS -----
    
    #[test]
    fn element_is_in_list() {
        let a: Vec<u8> = vec![8,2,1,3,4];
        let e = 2;
        assert!(permu_utils::is_in(&e, &a), "Failed to check if {} is in {:?}", e, a);

        let a: Vec<usize> = vec![700,3726,4163];
        let e = 4163;
        assert!(permu_utils::is_in(&e, &a), "Failed to check if {} is in {:?}", e, a);

        let a: Vec<f64> = vec![1.1,3.87,4.9];
        let e = 1.1;
        assert!(permu_utils::is_in(&e, &a), "Failed to check if {} is in {:?}", e, a);
        let e = 1.2;
        assert!(!permu_utils::is_in(&e, &a), "Failed to check if {} is in {:?}", e, a);
    }
    
    #[test]
    fn identity() {
        let mut a: Vec<u8> = vec![0;4];
        permu_utils::identity(&mut a);
        assert_eq!(a, vec![0,1,2,3], "Cannot generate a identity vector")
    }

    #[test]
    #[should_panic] 
    fn all_permutations_checker_panic() {
        let pop: Vec<Vec<u8>> = vec![vec![0,1,2], vec![2,1,3], vec![0,0,0]];
        permu_utils::all_permus(&pop, true);
    }

    #[test]
    fn all_permutations_checker_no_panic() {
        let pop: Vec<Vec<u8>> = vec![vec![0,1,2], vec![2,1,0], vec![2,0,1]];
        permu_utils::all_permus(&pop, true);
    }

    #[test]
    fn random_permutation_generator() {
        let size = 20;
        for _i in 0..2000 {
            let p = permu_utils::random_permutation(&size);
            let pop = vec![p];
            permu_utils::all_permus(&pop, true);
        }
    }

    #[test]
    fn generate_random_popualtion() {
        let pop = permu_utils::random_population(&10, &3000);
        permu_utils::all_permus(&pop, true);
    }

    #[test]
    fn transformations_permu_vj() {
        let size = 10;
        for _i in 0..2000 {
            let permu = permu_utils::random_permutation(&size); 
           
            let mut vj: Vec<u8> = vec![0; size-1];
            permu_utils::permu2vj(&permu, &mut vj);

            let mut permu2: Vec<u8> = vec![0; size];
            permu_utils::vj2permu(&vj, &mut permu2);

            assert_eq!(permu, permu2, "Failed transforming a permutation to vj");

            let pop = vec![permu];
            permu_utils::all_permus(&pop, true);
        }
    }
    
    #[test]
    fn discard_repeated_same_pop() {

        let mut discard: Vec<usize> = vec![];
        let correct: Vec<usize> = vec![2];

        let pop: Vec<Vec<u8>> = vec![
                        vec![1,2,0], vec![0,1,2],
                        vec![1,2,0], vec![0,2,1],
                        vec![2,1,0]];
        let pop_f: Vec<usize> = vec![10,12,10,12,7];

        permu_utils::discard_repeated_same_pop(&pop, &pop_f, 
                                               &mut discard);

        assert_eq!(discard, correct, "Discarding repeated samples from a single population failed.");
    }

    #[test]
    fn discard_repeated_different_pop() {
        let pop1: Vec<Vec<u8>> = vec![vec![0,1,2],
                                      vec![1,2,0],
                                      vec![0,1,2],
                                      vec![2,0,1]];
        let pop_f1 = vec![5,3,5,1];
        let pop2: Vec<Vec<u8>> = vec![vec![2,1,0],
                                      vec![1,0,2],
                                      vec![0,1,2],
                                      vec![1,2,0]];
        let pop_f2 = vec![1,5,5,3];
        let mut discard = vec![2];

        permu_utils::discard_repeated_different_pop(&pop1, &pop_f1,
                                                    &pop2, &pop_f2,
                                                    &mut discard);
        assert_eq!(discard, vec![2,0,1], 
                   "Failed to discard repeted solutions of pop1 in pop2");
        
        // Second test
        let pop1 = vec![vec![0,1,2],
                        vec![1,2,0],
                        vec![2,0,1]];
        let pop_f1 = vec![3,2,5];
        let pop2 = vec![vec![2,0,1],
                        vec![2,1,0],
                        vec![0,1,2]];
        let pop_f2 = vec![5,3,3];
        let mut discard = vec![];

        permu_utils::discard_repeated_different_pop(&pop1, &pop_f1,
                                                    &pop2, &pop_f2,
                                                    &mut discard);
        assert_eq!(discard, vec![0,2], 
                   "Failed to discard repeted solutions of pop1 in pop2");
    }

    #[test]
    fn invert_permutation_and_population() {

        let permu: Vec<u8> = vec![0,2,3,1];
        let mut permu_mut = permu.clone();

        permu_utils::invert(&mut permu_mut);
        assert_eq!(permu_mut, vec![0,3,1,2], "Failed to invert a permutation");

        permu_utils::invert(&mut permu_mut);
        assert_eq!(permu_mut, permu, "Failed to recover original permutation from inverse");
        
        // Now, invert and recover a poplation of permutations twice
        let mut pop = permu_utils::random_population(&20, &200);
        let pop_copy = pop.clone();

        for _i in 0..4 {
            permu_utils::invert_pop(&mut pop);
        } 

        assert_eq!(pop, pop_copy, "Failed to invert and recover a population of permutations");

    }

    #[test]
    fn argmin_argmax_argsort() {

        // Argmin
        let v = vec![1,4,2,4,2,1,0,3,5];

        assert_eq!(6, permu_utils::argmin(&v), 
                   "Failed to find the index of the minimum value from a vector.");
        assert_eq!(8, permu_utils::argmax(&v),
                    "Failed to find the index of the maximum value from a vector.");

        // Argsort 1
        let v = vec![2,1,0,3,5];
        let sorted = permu_utils::argsort(&v);

        assert_eq!(sorted,
                   vec![2,1,0,3,4],
                   "Failed to argsort a vector");        
        
        // Argsort 2
        let v = vec![1,1,0,0,5];
        let sorted = permu_utils::argsort(&v);

        assert_eq!(sorted,
                   vec![2,3,0,1,4],
                   "Failed to argsort a vector");        
    }
    
    #[test]
    fn vector_metrics() {
        let a = vec![1,3,2,4,1];
        
        /*
        assert_eq!(2.2, permu_utils::mean(&a), "Error calculating mean");
        assert_eq!(2, permu_utils::median(&a), "Error calculating median");
        assert_eq!(1, permu_utils::vec_min(&a), "Error finding minimum value");
        assert_eq!(4, permu_utils::vec_max(&a), "Error finding maximum value");
        */

        assert_eq!(Ok(1.16619037896906), permu_utils::std(&a), 
            "Error calculating standard deviation");

        assert_eq!((2.2,1,2,4), permu_utils::vec_metrics(&a),
            "Failed to calculate mean, min, meadian and max of a vector");
    }

    #[test]
    fn probability() {
        let d = vec![vec![2,1,0], vec![1,2,0], vec![0,0,3]];
        let fashion = vec![0,1,2];
        let p = permu_utils::probability(&d, &fashion);

        assert_eq!(0.4444444444444444, p);
    }

    // ----- PROBLEMS -----

    #[test]
    fn evaluation_qap() {
        let permu: Vec<u8> = vec![12,6,18,16,7,2,5,3,14,0,13,9,15,1,8,10,4,19,17,11];
        let instance = problems::qap::load_instance(".testing/tai20b.dat")
            .unwrap();
        assert_eq!(problems::qap::evaluate(&permu, &instance), 125551590);
    }

    #[test]
    fn evaluation_pfsp() {
        let permu: Vec<u8> = vec![2,16,8,14,13,7,18,12,15,5,6,0,1,3,4,17,19,11,10,9];
        let instance = problems::pfsp::load_instance(".testing/tai20_5_0.fsp")
            .unwrap(); 
        assert_eq!(problems::pfsp::evaluate(&permu, &instance), 14033);
    }
}
