#ifndef UTILS_H
#define UTILS_H

#include <string>
#include <vector>
using namespace std;

#define MINUS_INF (-1.L/0.L)
#define PLUS_INF (1.L/0.L) 

typedef long double ldouble; //sizeof(long double) is 16 on my Ubuntu 18.04

void printPerm(int* x, int n, string name="x");
void printValues(ldouble* v, int n, string name="w");
void printVectorInt(vector<int> v, string name="w");
bool isValidPerm(int* x, int n);
void pinv(int* y, int* x, int n); //y will be the inverse permutation of x
int enumeratedLongDoubleComparator(const void* a, const void* b);
int enumeratedIntComparator(const void* a, const void* b);
void softmax(ldouble* p, ldouble* w, int n); //p <- softmax(w)
void normalize(ldouble* p, ldouble* w, int n); //p <- normalize(w)
ldouble min(ldouble* v, int n);
ldouble max(ldouble* v, int n);
ldouble norm(ldouble* v, int n);
bool isInfNan(ldouble v);
void copyPerm(int* dst, int* src, int n);

float CalculateAvgKendallDistance(int ** samples_pool,int number_samples, int n,int*aux, int*inverted, int*composition, int*v);
#endif
