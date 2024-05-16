#ifndef PLACKETTLUCE_H
#define PLACKETTLUCE_H

#include "utils.h"
#include <cfloat>


struct PlackettLuce {

	int n;
	ldouble* w = 0;
	ldouble* expw = 0;
	ldouble sexpw; //sum(expw)
    ldouble entropy;
    
	PlackettLuce(int n);
	~PlackettLuce();

	void setUniformParameters();
	void setDegenerateParameters(int* mode);

	void sample(int* x);
    void sample_fast(int* x);
	bool gradLogProb(ldouble* g, int* x); //return false if there were numerical problems (inf or nan), true if everything ok
	void updateInnerParameters();

	void mode(int* x);
	ldouble entropyParameters();
	ldouble maxProb();
	ldouble prob(int* x);
	ldouble logProb(int* x);

	void print();

};

#endif
