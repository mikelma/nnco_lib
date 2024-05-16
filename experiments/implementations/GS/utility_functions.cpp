#include "utility_functions.h"
#include "utils.h"
#include "LopInstance.h"
#include <cmath>
#include <cstdlib>
#include <string>
#include <iostream>
using namespace std;


UtilityFunction* UtilityFunction::getUtilityFunction(string utilityFunction, int** x, int* fx, int lambda, LopInstance* lop) {
	if (utilityFunction=="fitness")
		return new FitnessUtilityFunction(x,fx,lambda,lop);
	else if (utilityFunction=="normalizedFitness")
		return new NormalizedFitnessUtilityFunction(x,fx,lambda,lop);
	else if (utilityFunction=="superlinear")
		return new SuperLinearUtilityFunction(x,fx,lambda,lop);
	else if (utilityFunction=="linear")
		return new LinearUtilityFunction(x,fx,lambda,lop);
	else if (utilityFunction=="equal")
		return new EqualUtilityFunction(x,fx,lambda,lop);
	else {
		cerr << "ERROR: The utility function \"" << utilityFunction << "\" does not exist!\n";
		exit(EXIT_FAILURE);
		return 0; //just to avoid compiler error
	}
}

UtilityFunction::UtilityFunction(int** x, int* fx, int lambda, LopInstance* lop) {
	this->x = x;
	this->fx = fx;
	this->lambda = lambda;
	this->lop = lop;
}

UtilityFunction::~UtilityFunction() {
	//do nothing
}



//BEGIN FITNESS
FitnessUtilityFunction::FitnessUtilityFunction(int** x, int* fx, int lambda, LopInstance* lop) : UtilityFunction(x,fx,lambda,lop) {
	//do nothing else
}

ldouble FitnessUtilityFunction::utility(int i) {
	return (fx[i]/(ldouble)lop->ub) * (1.L/lambda); //scaled by instance upper bound
}
//END FITNESS



//BEGIN FITNESS
NormalizedFitnessUtilityFunction::NormalizedFitnessUtilityFunction(int** x, int* fx, int lambda, LopInstance* lop) : UtilityFunction(x,fx,lambda,lop) {
	//do nothing else
}

void NormalizedFitnessUtilityFunction::preUtility() {
	fxMin = fx[0];
	for (int i=1; i<lambda; i++)
		if (fx[i]<fxMin) fxMin = fx[i];
	fxSum = 0.L;
	for (int i=0; i<lambda; i++)
		fxSum += fx[i]/fxMin; //dividing here and in utility() makes it mathematically equivalent to fx[i]/fxSum, but it is numerically better because we sum big numbers
}

ldouble NormalizedFitnessUtilityFunction::utility(int i) {
	return (fx[i]/fxMin) / fxSum; //see previous comment
}
//END FITNESS



//BEGIN SUPERLINEAR
SuperLinearUtilityFunction::SuperLinearUtilityFunction(int** x, int* fx, int lambda, LopInstance* lop) : UtilityFunction(x,fx,lambda,lop) {
	//implemented as in CMA-ES implementation in DEAP
	mu = lambda/2;
	weights = new ldouble[mu];
	ldouble logmu = log(mu+0.5L);
	ldouble s = 0.L;
    int k;
	for (k=0; k<mu; k++)
		s += weights[k] = logmu - log(k+1.L);
	for (k=0; k<mu; k++)
		weights[k] /= s;
	pos = new int[lambda];
}

SuperLinearUtilityFunction::~SuperLinearUtilityFunction() {
	if (weights) delete[] weights;
	if (pos) delete[] pos;
}

void SuperLinearUtilityFunction::preUtility() {
	//set efx to "enumerate(fx)"
	pair<int,int> efx[lambda];
	for (int i=0; i<lambda; i++) {
		efx[i].first = i;
		efx[i].second = fx[i];
	}
	//sort descending efx by fitness
	qsort(efx,lambda,sizeof(pair<int,int>),enumeratedIntComparator);
	//invert efx in pos, so pos[i] is the ranking of i-th sample
	for (int i=0; i<lambda; i++)
		pos[efx[i].first] = i;
	//done
}

ldouble SuperLinearUtilityFunction::utility(int i) {
	return pos[i]<mu ? weights[pos[i]] : 0.L;
}
//END SUPERLINEAR



//BEGIN LINEAR
LinearUtilityFunction::LinearUtilityFunction(int** x, int* fx, int lambda, LopInstance* lop) : UtilityFunction(x,fx,lambda,lop) {
	//implemented as in CMA-ES implementation in DEAP
	mu = lambda/2;
	weights = new ldouble[mu];
	ldouble s = 0.L;
	for (int k=0; k<mu; k++)
		s += weights[k] = mu + 0.5L - (k + 1.L);
	for (int k=0; k<mu; k++)
		weights[k] /= s;
	pos = new int[lambda];
}

LinearUtilityFunction::~LinearUtilityFunction() {
	if (weights) delete[] weights;
	if (pos) delete[] pos;
}

void LinearUtilityFunction::preUtility() {
	//set efx to "enumerate(fx)"
	pair<int,int> efx[lambda];
	for (int i=0; i<lambda; i++) {
		efx[i].first = i;
		efx[i].second = fx[i];
	}
	//sort descending efx by fitness
	qsort(efx,lambda,sizeof(pair<int,int>),enumeratedIntComparator);
	//invert efx in pos, so pos[i] is the ranking of i-th sample
	for (int i=0; i<lambda; i++)
		pos[efx[i].first] = i;
	//done
}

ldouble LinearUtilityFunction::utility(int i) {
	return pos[i]<mu ? weights[pos[i]] : 0.L;
}
//END LINEAR



//BEGIN EQUAL
EqualUtilityFunction::EqualUtilityFunction(int** x, int* fx, int lambda, LopInstance* lop) : UtilityFunction(x,fx,lambda,lop) {
	//implemented as in CMA-ES implementation in DEAP
	mu = lambda/2;
	pos = new int[lambda];
}

EqualUtilityFunction::~EqualUtilityFunction() {
	if (pos) delete[] pos;
}

void EqualUtilityFunction::preUtility() {
	//set efx to "enumerate(fx)"
	pair<int,int> efx[lambda];
	for (int i=0; i<lambda; i++) {
		efx[i].first = i;
		efx[i].second = fx[i];
	}
	//sort descending efx by fitness
	qsort(efx,lambda,sizeof(pair<int,int>),enumeratedIntComparator);
	//invert efx in pos, so pos[i] is the ranking of i-th sample
	for (int i=0; i<lambda; i++)
		pos[efx[i].first] = i;
	//done
}

ldouble EqualUtilityFunction::utility(int i) {
	return pos[i]<mu ? (1.L/mu) : 0.L;
}
//END EQUAL
