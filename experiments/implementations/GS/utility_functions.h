#ifndef UTILITY_FUNCTIONS_H
#define UTILITY_FUNCTIONS_H

#include "LopInstance.h"
#include "utils.h"
#include <string>
using namespace std;



struct UtilityFunction {
	int** x;
	int* fx;
	int lambda;
	LopInstance* lop;

	static UtilityFunction* getUtilityFunction(string utilityFunction, int** x, int* fx, int lambda, LopInstance* lop);

	UtilityFunction(int** x, int* fx, int lambda, LopInstance* lop);
	virtual ~UtilityFunction();

	virtual void preUtility() {};
	virtual ldouble utility(int i) = 0;
};



struct FitnessUtilityFunction : public UtilityFunction {
	FitnessUtilityFunction(int** x, int* fx, int lambda, LopInstance* lop);
	virtual ldouble utility(int i);
};



struct NormalizedFitnessUtilityFunction : public UtilityFunction {
	ldouble fxSum,fxMin;
	NormalizedFitnessUtilityFunction(int** x, int* fx, int lambda, LopInstance* lop);
	virtual void preUtility();
	virtual ldouble utility(int i);
};



struct SuperLinearUtilityFunction : public UtilityFunction {
	int mu;
	ldouble* weights = 0;
	int* pos = 0;

	SuperLinearUtilityFunction(int** x, int* fx, int lambda, LopInstance* lop);
	virtual ~SuperLinearUtilityFunction();

	virtual void preUtility();
	virtual ldouble utility(int i);
};



struct LinearUtilityFunction : public UtilityFunction {
	int mu;
	ldouble* weights = 0;
	int* pos = 0;

	LinearUtilityFunction(int** x, int* fx, int lambda, LopInstance* lop);
	virtual ~LinearUtilityFunction();

	virtual void preUtility();
	virtual ldouble utility(int i);
};



struct EqualUtilityFunction : public UtilityFunction {
	int mu;
	int* pos = 0;

	EqualUtilityFunction(int** x, int* fx, int lambda, LopInstance* lop);
	virtual ~EqualUtilityFunction();

	virtual void preUtility();
	virtual ldouble utility(int i);
};

#endif
