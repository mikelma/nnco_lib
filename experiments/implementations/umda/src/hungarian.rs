extern crate pathfinding;
// use pathfinding::kuhn_munkres;
use pathfinding::kuhn_munkres::Weights;

pub struct W {
    matrix : Vec<Vec<isize>>,
}

impl W {
    pub fn from(m: &Vec<Vec<usize>>) -> W {
        let mut v: Vec<Vec<isize>> = vec![];

        for row in m {
            let mut l = vec![];
            for e in row {
                l.push(*e as isize);
            }
            v.push(l);
        }
        W {matrix : v}
    }

    pub fn hungarian(&self) -> Vec<u8> {
        let weights = W { matrix : self.matrix.clone() };
        let result = pathfinding::kuhn_munkres::kuhn_munkres(&weights);
        let mut v: Vec<u8> = vec![]; 
        for e in result.1 {
            v.push(e as u8); 
        }
        v
    }
}

impl Weights<isize> for W {
    fn rows(&self) -> usize {
        self.matrix.len()
    }
    fn columns(&self) -> usize {
        self.matrix[0].len()
    }
    fn at(&self, row: usize, col: usize) -> isize {
        self.matrix[row][col]
    }
    fn neg(&self) -> W {
        let mut r: Vec<Vec<isize>> = vec![vec![0;self.columns()];self.rows()];
        for i in 0..self.rows() {
            for j in 0..self.columns() {
                r[i][j] = -self.matrix[i][j];
            }
        }
        W { matrix : r }
    } 
}

#[cfg(test)]
mod test {

    #[test]
    fn hungarian() {
        use super::W;

        let a = vec![vec![1,3,4],
                     vec![2,3,1],
                     vec![4,1,5]];
         
        let test = W::from(&a);
        let test = test.hungarian();
        assert_eq!(test, vec![2,1,0]);
    }
}
