#ifndef ALGORITHMS_H
#define ALGORITHMS_H

#include "utils.h"
#include "LopInstance.h"
#include "Variables.h"
#include <Eigen/Dense>

int* gradientAscent_PL(string lopInstanceName, LopInstance& lop, int RUNNING_MODE, ldouble alpha, int lambda, int maxEvaluations=10000, string utilityFunction="superlinear", int printInterval=0, unsigned int seed=1,ldouble lowerAlpha=1e-4, ldouble upperAlpha=0.9);

int* naturalgradientAscent_PL(string lopInstanceName, LopInstance& lop, int RUNNING_MODE, ldouble alpha, int lambda, int maxEvaluations=10000, string utilityFunction="superlinear", int printInterval=0, unsigned int seed=1,ldouble lowerAlpha=1e-4, ldouble upperAlpha=0.9);

int* gradientAscent_PL_selfadjusting_learning(string lopInstanceName, LopInstance& lop, int RUNNING_MODE, int lambda=100, ldouble initialalpha=0.01, int maxEvaluations=10000,  string utilityFunction="superlinear", int printInterval=0, unsigned int seed=1,ldouble lowerAlpha=1e-4, ldouble upperAlpha=0.9);

int* gradientAscent_PL_selfadjusting_sample(string lopInstanceName, LopInstance& lop,int RUNNING_MODE,int initialLambda, int lowerlambda=10, int upperlambda=10000, ldouble alpha=0.01, int maxEvaluations=10000,  string utilityFunction="superlinear", int printInterval=0, unsigned int seed=1,ldouble lowerAlpha=1e-4, ldouble upperAlpha=0.9);

int* gradientAscent_PL_selfadjusting_total(string lopInstanceName, LopInstance& lop,int RUNNING_MODE, int initialLambda, int lowerlambda=10, int upperlambda=10000, ldouble initialalpha=0.01, int maxEvaluations=10000,  string utilityFunction="superlinear", int printInterval=0, unsigned int seed=1,ldouble lowerAlpha=1e-4, ldouble upperAlpha=0.9);

int* naturalgradientAscent_PL_selfadjusting_total(string lopInstanceName, LopInstance& lop,int RUNNING_MODE, int initialLambda, int lowerlambda=10, int upperlambda=10000, ldouble initialalpha=0.01, int maxEvaluations=10000,  string utilityFunction="superlinear", int printInterval=0, unsigned int seed=1,ldouble lowerAlpha=1e-4, ldouble upperAlpha=0.9);

int* gradientAscent_BT(string lopInstanceName, LopInstance& lop,int RUNNING_MODE, ldouble alpha, int lambda, int maxEvaluations=10000, int sampling_iterations=100, string utilityFunction="superlinear", int printInterval=0, unsigned int seed=1,ldouble lowerAlpha=1e-4, ldouble upperAlpha=0.9);

#endif
