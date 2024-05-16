#include "BradleyTerry.h"
#include "random.h"
#include "utils.h"
#include <cmath>
#include <cstring>
#include <cstdlib>
#include <iostream>
#include <utility>
#include <vector>
#include <algorithm>
#include <climits>
using namespace std;


BradleyTerry::BradleyTerry(int n) {
    this->n = n;
    w = new ldouble[n];
    expw = new ldouble[n];
    //matrix_expw=new ldouble*[n];
    precomputed_matrix=new ldouble*[n];
    Q_vector = new ldouble[n];
    for (int i=0;i<n;i++){
      //  matrix_expw[i]= new ldouble[n];
        precomputed_matrix[i]= new ldouble[n];
    }
    setUniformParameters();
    
    normalization_constant=0;
     
    sample.reserve(n);
    remInd.reserve(n);
}


BradleyTerry::~BradleyTerry() {
    if (w) delete[] w;
    if (expw) delete[] expw;
    if (Q_vector) delete[] Q_vector;
    if (matrix_expw){
        for (int i=0;i<n;i++){
            delete [] matrix_expw[i];
            delete [] precomputed_matrix[i];
        }
        delete [] matrix_expw;
        delete [] precomputed_matrix;
    }
    sample.clear();
    remInd.clear();
}


void BradleyTerry::setUniformParameters() {
    ldouble u = 1.L/n; //1.L; //M_E;
    ldouble logu = log(u); //0.L; //1.L;
    int i,j;
    for (i=0; i<n; i++) {
        w[i] = logu;
        expw[i] = u;
    }
   // ldouble value;
     /*for (i=0;i<n;i++){
         for (j=i+1;j<n;j++){
             value=expw[i] + expw[j];
             matrix_expw[i][j]=value;
             matrix_expw[j][i]=value;
         }
     }*/
     for (i=0;i<n;i++){
         precomputed_matrix[i][i]=0;
         for (j=i+1;j<n;j++){
             precomputed_matrix[i][j]=expw[j]/(expw[i] + expw[j]);
             precomputed_matrix[j][i]=1-precomputed_matrix[i][j];
         }
     }
    for (i=0;i<n;i++){
        Q_vector[i]=0;
        for (j=0;j<n;j++){
            Q_vector[i]+=precomputed_matrix[i][j];
        }
    }
    sexpw = 1.L; //n; //n*M_E;
}

void BradleyTerry::setRandomParameters(){

    int randomNumber;
    ldouble sumNumbers=0;
    int index,j;
    double val=rand();
    double lambda=((double) rand() / (INT_MAX));
   
    //cout<<"lambda: "<<lambda<<endl;
      for (index = 0; index < n; index++) {
        randomNumber = (rand() % n) + 1;
          expw[index]=exp(-lambda*randomNumber);
          sumNumbers+=expw[index];
      }
    
  
    for (index = 0; index < n; index++) {
        expw[index]=expw[index]/sumNumbers;
        w[index]=log(expw[index]);
    }
   // printValues(expw, n); exit(1);
     for (index=0;index<n;index++){
         precomputed_matrix[index][index]=0;
         for (j=index+1;j<n;j++){
             precomputed_matrix[index][j]=expw[j]/(expw[index] + expw[j]);
             precomputed_matrix[j][index]=1-precomputed_matrix[index][j];
         }
     }
    for (index=0;index<n;index++){
        Q_vector[index]=0;
        for (j=0;j<n;j++){
            Q_vector[index]+=precomputed_matrix[index][j];
        }
    }
    sexpw = 1.L; //n; //n*M_E;
}

void BradleyTerry::setDegenerateParameters(int* mode) {
    //upper and lower bounds and range for the weights
    const ldouble ub=+10.L, lb=-10.L;
    //compute step size
    ldouble step = (ub-lb)/(n-1);
    //assign weights such that first has weight ub and last has weight lb
    w[mode[0]] = ub;
    sexpw = expw[mode[0]] = exp(ub);
    int i,j;
    for (i=1; i<n; i++) {
        w[mode[i]] = w[mode[i-1]]-step;
        sexpw += expw[mode[i]] = exp(w[mode[i]]);
    }
    //ldouble value;
    /*   for (i=0;i<n;i++){
           for (j=i+1;j<n;j++){
               value=expw[i] + expw[j];
               matrix_expw[i][j]=value;
               matrix_expw[j][i]=value;
           }
       }*/
    for (i=0;i<n;i++){
            precomputed_matrix[i][i]=0;
            for (j=i+1;j<n;j++){
                precomputed_matrix[i][j]=expw[j]/(expw[i] + expw[j]);
                precomputed_matrix[j][i]=1-precomputed_matrix[i][j];
            }
        }
    for (i=0;i<n;i++){
          Q_vector[i]=0;
          for (j=0;j<n;j++){
              Q_vector[i]+=precomputed_matrix[i][j];
          }
      }
    //done
}

void BradleyTerry::sample_heuristic_classical(int* x, int max_iterations) {
    
    //XXXXX: code to be compiled and debugged.
    float probability, prob;
    bool stop=false;
    
    //Create a vector of the elements to be placed in the permutation.
    for (int i=0; i<n; i++)
        remInd.push_back(i);
    
    // chose an element from R at random and insert it in x.
    int r = rand() %n;
    sample.push_back(remInd[r]);
    remInd.erase(remInd.begin()+r);
       
   // printVectorInt(sample, "sample");
   // printVectorInt(remInd, "remInd");
    int z, item_r, item_s, iterations;
    //while(remInd.empty()==false){
    for (int items_sampled=1;items_sampled<n;items_sampled++){
        //srand(time(NULL));   // Initialization, should only be called once.
        r = rand() %(n-items_sampled);
        item_r=remInd[r];
        
        z=rand() %items_sampled;
        item_s=sample[z];
    
        iterations=0;
        stop=false;
       // cout<<"item_r: "<<item_r<<" item_s: "<<item_s<<endl;
        while (iterations<max_iterations && stop==false){ //an stopping criterion need to be set to avoid infinite loop
            //probability=expw[item_r]/(expw[item_r]+expw[item_s]);
            //probability=expw[item_r]/matrix_expw[item_r][item_s]; //optimized
            probability=precomputed_matrix[item_s][item_r]; //super-optimized
            
            prob = ((double) rand() / (RAND_MAX));
            //cout<<"probability: "<<probability<<" prob: "<<prob<<endl;
            
            stop=false;
            if (prob<probability){
                z--;
                if (z<0) { z=0; }//avoid taking negative values.
                else{
                    item_s=sample[z];
                }
            }
            else{
               // cout<<"old z: "<<z<<"new z: "<<z+1<<" sample size: "<<sample.size()<<endl;
                z++;
                if (z==items_sampled) {z=items_sampled; stop=true;} //avoid taking values out of the range. last position is possible but not more.
                else{
                    item_s=sample[z];
                }
            }
            iterations++;
        }
       // cout<<"to insert item "<<item_r<<" in position "<<z<<" of the sample"<<endl;
        
        sample.insert(sample.begin()+z,item_r);
   //     printVectorInt(sample, "sample");
                
        remInd.erase(remInd.begin()+r);
            
    }
    std::copy(sample.begin(), sample.end(), x);
    sample.clear();
    
//  printPerm(x,n,"x");

}

void BradleyTerry::sample_solutions (int num_solutions, int ** samples, int n, int max_iterations){
    int i,j=0;
    for (i=0;i<num_solutions;i++){
        for (j=0;j<n;j++){
            sample_heuristic_classical(samples[i],max_iterations);
        }
    }
}

bool BradleyTerry::gradLogProb_classical(ldouble* g, int* x) {
    int i,j;
    ldouble tosubstract,tosum;
    for (i=0;i<n;i++){
        tosubstract=0;
        tosum=0;
        for (j=0;j<i-1;j++)
            tosubstract+=((ldouble)1)/(w[x[j]]+w[x[i]]);
        for (j=i+1;j<n;j++)
            tosum+=(w[x[j]])/(w[x[j]]*w[x[i]]-pow(w[x[i]],2));
        g[x[i]]=tosum-tosubstract;
    }
    return true; //if here, everything was ok
}

bool BradleyTerry::gradLogProb_exponential(ldouble* g, int* x) {
    int i,j;
    ldouble tosubstract,tosum;
    for (i=0;i<n;i++){
        tosubstract=0; tosum=0;
        for (j=0;j<=(i-1);j++){
            //tosubstract+=(expw[x[i]])/(expw[x[j]]+expw[x[i]]); //non-optimized
            tosubstract+=(expw[x[i]])/(matrix_expw[x[j]][x[i]]); //optimized
        }
        for (j=i+1;j<n;j++){
            //tosum+=(expw[x[j]])/(expw[x[j]]+expw[x[i]]);//non-optimized
           tosum+=(expw[x[j]])/(matrix_expw[x[j]][x[i]]);//optimized
        }
        //cout<<"tosum: "<<tosum<<" tosubstract: "<<tosubstract<<endl;
        g[x[i]]=tosum-tosubstract;
    }
    return true; //if here, everything was ok
}

bool BradleyTerry::gradLogProb_exponential_efficient(ldouble* g, int* x) {
    for (int i=0;i<n;i++){
       // cout<<"Q_vector: "<<Q_vector[x[i]]<<endl;
        g[x[i]]=1+Q_vector[x[i]]-i;//According to XXX's code, 1 unit should be added.
    }
    return true; //if here, everything was ok
}

// computing the gradient this way, implies calculating the probability of the permutation according to the bradley-terry model (taking into account the normalization constant too).
bool BradleyTerry::gradLogProb_exhaustive(ldouble* g, int* x) {
    int i;
    ldouble tosum, tosubstract, value;
    ldouble h= 0.000001;
    for (i=0;i<n;i++)
    {
        expw[i]= exp(w[i]+h);
        tosum=log(this->prob_exponential(x));
        expw[i]=exp(w[i]-(2*h));
        tosubstract=log(this->prob_exponential(x));
        expw[i]=exp(w[i]+h);
        value=(tosum-tosubstract)/(2*h);
        cout<<"tosum: "<<tosum<<" tosubstract: "<<tosubstract<<" value: "<<value<<endl;
        g[i]=value;
    }
    return true; //if here, everything was ok
}

void BradleyTerry::updateInnerParameters() {
    //BASE VERSION: since w weights were updated by gradientAscent, recompute expw and sexpw
    sexpw = 0.L;
    int i,j;
    for (i=0; i<n; i++){
        sexpw += expw[i] = exp(w[i]);
    }
    /*ldouble value;
    for (i=0;i<n;i++){
        for (j=i+1;j<n;j++){
            value=expw[i] + expw[j];
            matrix_expw[i][j]=value;
            matrix_expw[j][i]=value;
        }
    }*/
    for (i=0;i<n;i++){
            precomputed_matrix[i][i]=0;
            for (j=i+1;j<n;j++){
                precomputed_matrix[i][j]=expw[j]/(expw[i] + expw[j]);
                precomputed_matrix[j][i]=1-precomputed_matrix[i][j];
            }
        }
    
    
    for (i=0;i<n;i++){
          Q_vector[i]=0;
          for (j=0;j<n;j++){
              Q_vector[i]+=precomputed_matrix[i][j];
          }
      }
    /* COMMENTATA PERCHE' NON FUNZIONA DAVVERO MEGLIO DELLA VERSIONE BASE
    //VERSION WITH SOFTMAX TRICK: w weights were updated by gradientAscent: adjust them by subtracting max-weight and recompute expw and sexpw
    ldouble maxw = max(w,n);
    sexpw = 0.L;
    for (int i=0; i<n; i++) {
        w[i] -= maxw;
        sexpw += expw[i] = exp(w[i]);
    }
    */
}


void BradleyTerry::mode(int* x) {
    //set ww to "enumerate(w)" (Python style)
    pair<int,ldouble> ww[n];
    for (int i=0; i<n; i++) {
        ww[i].first = i;
        ww[i].second = w[i];
    }
    //sort ww by weights
    qsort(ww,n,sizeof(pair<int,ldouble>),enumeratedLongDoubleComparator);
    //THERE WAS A BUG HERE (MODE IS THE SORTING INDEXES AND NOT THEIR INVERSE) ... mode is now the inversion of the sorted indexes in ww
    for (int i=0; i<n; i++)
        x[i] = ww[i].first; //THE BUGGED LINE WAS: x[ww[i].first] = i;
    //done
}


ldouble BradleyTerry::entropyParameters() {
    ldouble entr=0.L, p;
    for (int i=0; i<n; i++) {
        p = expw[i]/sexpw;
        entr += -p*log(p);
    }
    return entr;
}


ldouble BradleyTerry::maxProb() {
    ldouble expwMax = expw[0];
    for (int i=1; i<n; i++)
        if (expw[i]>expwMax) expwMax = expw[i];
    return expwMax/sexpw;
}


ldouble BradleyTerry::prob_exponential(int* x) {
    //inefficient implementation, but not used!!!
    int i,j;
    ldouble p=1.L;
    for (i=0; i<n-1; i++) { //n-1 or n is the same, bcs n-th factor is 1
        for (j=i+1;j<n;j++){
            p*=(expw[x[i]])/(expw[x[i]]+expw[x[j]]);
        }
    }
    //if (normalization_constant==0){
        normalization_constant=0;
        ldouble value=1.L;
        //calculate the normalizatio constant for the first time.
        int* perm= new int[n];
        for (int i=0;i<n;i++)
            perm[i]=i;
        sort(perm, perm + n);
        do {
            value=1.L;
            for (i=0; i<n-1; i++) { //n-1 or n is the same, bcs n-th factor is 1
                for (j=i+1;j<n;j++){
                    value*=(expw[perm[i]])/(expw[perm[i]]+expw[perm[j]]);
                }
            }
            normalization_constant+=value;
        } while (next_permutation(perm, perm + n));
        delete [] perm;
   // }
    p=p/normalization_constant;
    return p;
}


ldouble BradleyTerry::prob_classical(int* x) {
    //inefficient implementation, but not used!!!
    int i,j;
    ldouble p=1.L;
    for (i=0; i<n-1; i++) { //n-1 or n is the same, bcs n-th factor is 1
        for (j=i+1;j<n;j++){
            p*=(w[x[i]])/(w[x[i]]+w[x[j]]);
        }
    }
    //if (normalization_constant==0){
        normalization_constant=0;
        ldouble value=1.L;
        //calculate the normalizatio constant for the first time.
        int* perm= new int[n];
        for (int i=0;i<n;i++)
            perm[i]=i;
        sort(perm, perm + n);
        do {
            value=1.L;
            for (i=0; i<n-1; i++) { //n-1 or n is the same, bcs n-th factor is 1
                for (j=i+1;j<n;j++){
                    value*=(w[perm[i]])/(w[perm[i]]+w[perm[j]]);
                }
            }
            normalization_constant+=value;
        } while (next_permutation(perm, perm + n));
        delete [] perm;
   // }
    p=p/normalization_constant;
    return p;
}

ldouble BradleyTerry::prob_proportional(int* x) {
    int i,j;
    ldouble p=1.L;
    for (i=0; i<n-1; i++) { //n-1 or n is the same, bcs n-th factor is 1
        for (j=i+1;j<n;j++){
            p*=(expw[x[i]])/(expw[x[i]]+expw[x[j]]);
        }
    }
    return p;
}

void BradleyTerry::print() {
    cout << "*** BradleyTerry ***\n";
    cout << "n = " << n << "\n";
    printValues(w,n,"w");
    printValues(expw,n,"expw");
    cout << "sexpw = " << sexpw << "\n";
    int x[n];
    mode(x);
    printPerm(x,n,"mode");
    cout << "sizeof(ldouble) = " << sizeof(ldouble) << "\n";
    cout << "********************\n";
}
