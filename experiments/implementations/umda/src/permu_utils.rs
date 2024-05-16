use rand::Rng;

pub fn is_in<T: std::cmp::PartialEq>(elem: &T, vec: &Vec<T>) -> bool {

    let mut i = 0;
    let mut is = false;

    while i < vec.len() && !is {
        is = vec[i] == *elem;
        i += 1;
    }
    is
}

pub fn random_permutation(size: &usize) -> Vec<u8> {
    let mut permu: Vec<u8> = Vec::new(); 
    let max = *size as u8;
    
    while permu.len() < *size {

        let n = rand::thread_rng().gen_range(0, max);

        if !is_in(&n, &permu) {
            permu.push(n);
        }
    }
    permu
}

pub fn count_element(vec: &Vec<u8>, e: &u8) -> usize {
    vec.iter().filter(|&n| *n == *e).count()
}

pub fn vec_exists_in(vec: &Vec<u8>, pop: &Vec<Vec<u8>>) -> bool {
    let mut found = false;
    let mut i = 0;

    while i < pop.len() && !found {
        found = vec.eq(&pop[i]);
        i += 1;
    }
    found
}

pub fn random_population(lenght: &usize, pop_size: &usize) -> Vec<Vec<u8>> {
    let mut pop: Vec<Vec<u8>> = Vec::new();
    let mut i = 0;

    while i < *pop_size {
        let new = random_permutation(&lenght);        
        if !vec_exists_in(&new, &pop) {
            pop.push(new);
            i += 1;
        }
    }
    pop
} 

pub fn argmin(v: &Vec<usize>) -> usize {
    let mut indx = 0;
    for i in 0..v.len() {
        if v[i] < v[indx] {
            indx = i;
        }
    }
    indx
}

pub fn argmax(v: &Vec<usize>) -> usize {
    let mut indx = 0;
    for i in 0..v.len() {
        if v[i] > v[indx] {
            indx = i;
        }
    }
    indx
}

// TODO: Optimize function
pub fn argsort(vec: &Vec<usize>) -> Vec<usize> {
    
    let mut sorted = vec.clone();
    let mut argsort: Vec<usize> = vec![];

    sorted.sort();
    // sorted.sort_unstable();
    
    for e in &sorted {
        
        let mut found = false;
        let mut i = 0;
        while i < sorted.len() && !found {
            if *e == vec[i] && !is_in(&i, &argsort){ 
                found = true;
                argsort.push(i);
            }
            i += 1;
        }
    }
    argsort
}

pub fn permu2vj<'a>(permu: &Vec<u8>, vj: &'a mut Vec<u8>) {

    assert_eq!(permu.len()-1, vj.len(), "Lenght of the vj vector must be permu.len()-1");
    
    // Reset vj
    for i in 0..vj.len() {
        vj[i] = 0;
    }

    for indx in 0..permu.len() {
        for i in indx..permu.len() {
            if permu[i] < permu[indx] {
                vj[indx] += 1;
            }            
        }
    }
}

pub fn identity<'a>(v: &'a mut Vec<u8>) {
    for i in 0..v.len() {
        v[i] = i as u8; 
    }
}

pub fn vj2permu<'a>(vj: &Vec<u8>, permu: &'a mut Vec<u8>) {
   
    assert_eq!(permu.len(), vj.len()+1, "Lenght of the permu vector must be vj.len()+1");
    let s = permu.len();
    let mut e: Vec<u8> = vec![0;permu.len()];
    identity(&mut e);

    for (i, v) in vj.iter().enumerate() {
        permu[i] = e[*v as usize];
        e.remove(*v as usize);
    }
    permu[s-1] = e[0];
}

pub fn fancy_print<T: std::fmt::Debug>(m: &Vec<Vec<T>>, title: &str) {
    println!("{}", title);
    for e in m {
        println!("{:?}", e);
    }
}

pub fn discard_repeated_same_pop<'a>(pop: &Vec<Vec<u8>>, 
                                     pop_f: &Vec<usize>, 
                                     discard: &'a mut Vec<usize>) {

    // Find if i, f1 sample is repeated
    for (i, f1) in pop_f.iter().enumerate() {
        //let mut j: isize = i as isize;
        let mut j = i;
        let mut stop = false;

        while !stop {

            let f2 = pop_f[j];

            if i != j && *f1 == f2 && !is_in(&i, &discard) {
                if pop[i] == pop[j]{
                    discard.push(i);
                    // println!("{:?} and {:?} are equ", pop[i], pop[j]);
                    stop = true;
                }
            }
            if j == 0 {
                stop = true;
            } else {
                j -= 1;
            }
        }
    }
}
/// Discard repeated solutions of pop1 in pop2
pub fn discard_repeated_different_pop<'a>(pop1: &Vec<Vec<u8>>, 
                                          pop_f1: &Vec<usize>, 
                                          pop2: &Vec<Vec<u8>>, 
                                          pop_f2: &Vec<usize>,                                      
                                          discard: &'a mut Vec<usize>) {

    assert_eq!(pop1.len(), pop2.len(), "Both populations must have the same size.");

    // Find if i, f1 sample is repeated
    for (i, f1) in pop_f1.iter().enumerate() {
        //let mut j = i;
        let mut j = pop2.len()-1;
        let mut stop = false;

        while !stop {

            let f2 = pop_f2[j];

            if *f1 == f2 && !is_in(&i, &discard) {
                if pop1[i] == pop2[j]{
                    discard.push(i);
                    stop = true;
                }
            }
            if j == 0 {
                stop = true;
            } else {
                j -= 1;
            }
        }
    }
}

pub fn all_permus(pop: &Vec<Vec<u8>>,
                  panic: bool) -> bool {
    
    let mut i = 0;
    let mut all_permu = true;

    while i < pop.len() && all_permu {
        let permu = pop[i].clone();
        let mut e = 0;
        while e < permu.len() && all_permu {
            all_permu = count_element(&permu, &(e as u8)) ==  1;
            e += 1;
        }
        if panic && !all_permu {
            panic!("Found a non permutation solution: {:?}.", permu) 
        }
        i += 1;
    }
    all_permu
} 

pub fn invert<'a>(permu: &'a mut Vec<u8>) {

    let copy = permu.clone();

    for i in 0..permu.len() {
        permu[copy[i] as usize] = i as u8;
    } 
} 

pub fn invert_pop<'a>(pop: &'a mut Vec<Vec<u8>>) {
    
    for i in 0..pop.len() {
        invert(&mut pop[i]);
    }
}

pub fn divide_by(m: &Vec<Vec<usize>>, x: &usize) -> Vec<Vec<f64>> {
    let mut r: Vec<Vec<f64>> = vec![vec![0.0; m[0].len()]; m.len()];
    for i in 0..m.len() {
        for j in 0..m[i].len() {
            r[i][j] = m[i][j] as f64 / *x as f64;
        }
    }
    r
}

/// Discrete distribution to probability matrix
pub fn distr2prob(distr: &Vec<Vec<usize>>) -> Vec<Vec<f64>> {
    let mut prob = vec![];

    for row in distr {
        let mut new = vec![];
        let sum: usize = row.iter().sum();
        let sum = sum as f64;
        for elem in row {
            new.push(*elem as f64 / sum); 
        }
        prob.push(new);
    }
    prob
}

pub fn probability(distr: &Vec<Vec<usize>>, v: &Vec<u8>) -> f64 {
    // let pr = divide_by(distr, &distr[0].len());  // Probability matrix
    let pr = distr2prob(distr);
    let mut p = 1.0; // Probability of v to be sampled from distr
    for (i, e) in v.iter().enumerate() {
        p *= pr[i][*e as usize] ;
    }
    p
}

pub fn vec_metrics(vec: &Vec<usize>) -> (f64, usize, usize, usize) {
    // Calculate mean
    let sum: usize = vec.iter().sum();
    let mean = sum as f64 / vec.len() as f64;
    // Calculate minimum
    let mut sorted = vec.clone();
    sorted.sort();
    let min = sorted[0];
    // Calculate median
    let median = sorted[sorted.len()/2];
    //Calculate maximum
    let max = sorted[sorted.len()-1];

    (mean, min, median, max)
}

pub fn zeros_u8(rows: usize, cols: usize) -> Vec<Vec<u8>> {
    vec![vec![0;cols];rows]
}

pub fn std(v: &Vec<usize>) -> Result<f64, &'static str> {
    let s: usize = v.iter().sum();
    let mean = s as f64 / v.len() as f64;

    let mut dev :f64 = v.iter()
        .map(|x| {(*x as f64 - mean).powi(2)})
        .sum();
    
    dev /= v.len() as f64;
    dev = dev.sqrt();

    Ok(dev)
}

/*
pub fn shape<T>(matrx: &Vec<Vec<T>>) -> (usize, usize) {
    (matrx.len(), matrx[0].len())
}
*/
