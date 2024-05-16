#ifndef BRADLEYTERRY_H
#define BRADLEYTERRY_H

#include "utils.h"
#include <cfloat>


struct BradleyTerry {

	int n;
	ldouble* w = 0;
	ldouble* expw = 0;
	ldouble sexpw; //sum(expw)
    ldouble** matrix_expw = 0;
    ldouble** precomputed_matrix = 0;
    ldouble* Q_vector=0;
    ldouble normalization_constant;
    
    //Vectors needed for the heuristic sampling.
    vector<int> sample;
    vector<int> remInd;
    
	BradleyTerry(int n);
	~BradleyTerry();

	void setUniformParameters();
    void setRandomParameters();
	void setDegenerateParameters(int* mode);

	void sample_heuristic_classical(int* x, int max_iterations);
    void sample_solutions (int num_solutions, int ** samples, int n, int max_iterations);
    
	bool gradLogProb_classical(ldouble* g, int* x);
    bool gradLogProb_exponential(ldouble* g, int* x);
    bool gradLogProb_exponential_efficient(ldouble* g, int* x);
    bool gradLogProb_exhaustive(ldouble* g, int* x);//only to check that the derivative is calculated correctly.
    

	void updateInnerParameters();

	void mode(int* x);
	ldouble entropyParameters();
	ldouble maxProb();
	ldouble prob_exponential(int* x);
    ldouble prob_classical(int* x);
    ldouble prob_proportional(int* x); //without normalization constant and exponential verson.
	
	void print();

};

#endif
