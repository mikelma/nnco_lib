#include "utils.h"
#include <cstring>
#include <cmath>
#include <iostream>
using namespace std;


void printPerm(int* x, int n, string name) {
	cout << name << " = [ ";
	for (int i=0; i<n; i++)
		cout << x[i] << (i<n-1?",":"") << " ";
	cout << "]\n";
}

void printValues(ldouble* v, int n, string name) {
	cout << name << " = [ ";
	for (int i=0; i<n; i++)
		cout << v[i] << (i<n-1?",":"") << " ";
	cout << "]\n";
}

void printVectorInt(vector<int> vec, string name) {
    cout << name << " = [";
     for (std::vector<int>::iterator it = vec.begin(); it != vec.end(); ++it)
         std::cout << ' ' << *it;
    cout << "]\n";
}

bool isValidPerm(int* x, int n) {
	bool present[n];
	memset(present,0,sizeof(bool)*n);
	for (int i=0; i<n; i++) {
		if (present[x[i]]) return false;
		present[x[i]] = true;
	}
	for (int i=0; i<n; i++)
		if (!present[i]) return false;
	return true;
}


void pinv(int* y, int* x, int n) {
	for (int i=0; i<n; i++)
		y[x[i]] = i;
}


int enumeratedLongDoubleComparator(const void* a, const void* b) {
	int r = 0;
	ldouble diff = ((pair<int,ldouble>*)a)->second - ((pair<int,ldouble>*)b)->second;
	if (diff<0.L)
		r = +1; //sorting descending
	else if (diff>0.L)
		r = -1; //sorting descending
	return r;
}


int enumeratedIntComparator(const void* a, const void* b) {
	int r = 0;
	int diff = ((pair<int,int>*)a)->second - ((pair<int,int>*)b)->second;
	if (diff<0.L)
		r = +1; //sorting descending
	else if (diff>0.L)
		r = -1; //sorting descending
	return r;
}


void softmax(ldouble* p, ldouble* w, int n) {
	int i;
	ldouble s = 0.L;
	for (i=0; i<n; i++)
		s += p[i] = exp(w[i]);
	for (i=0; i<n; i++)
		p[i] /= s;
}


void normalize(ldouble* p, ldouble* w, int n) {
	int i;
	ldouble s = 0.L;
	for (i=0; i<n; i++)
		s += p[i] = w[i];
	for (i=0; i<n; i++)
		p[i] /= s;
}


ldouble min(ldouble* v, int n) {
	ldouble r = v[0];
	for (int i=1; i<n; i++)
		if (v[i]<r) r = v[i];
	return r;
}


ldouble max(ldouble* v, int n) {
	ldouble r = v[0];
	for (int i=1; i<n; i++)
		if (v[i]>r) r = v[i];
	return r;
}


ldouble norm(ldouble* v, int n) {
	ldouble r = 0.L;
	for (int i=0; i<n; i++)
		r += v[i]*v[i];
	return sqrt(r);
}


bool isInfNan(ldouble v) {
	return isinf(v) || isnan(v);
}


void copyPerm(int* dst, int* src, int n) {
	memcpy(dst,src,sizeof(int)*n);
}
/*
 * Inverts a permutation.
 */
void Invert(int*permu, int n, int* inverted)
{
    int i;
    for(i=0; i<n; i++)
        inverted[permu[i]]=i;
}

/*
 * Implements the compose of 2 permutations of size n.
 */
void Compose(int*s1, int*s2, int*res, int n)
{
    int i;
    for(i=0;i<n;i++)
        res[i]=s1[s2[i]];
}

/*
 *  Optimized version proposed by Leti for the calculation of the V_j-s vector.
 */
void vVector_Fast(int*v, int*permutation, int n, int * m_aux)
{
    int i,j, index;
    
    for(i=0;i<n-1;i++){
        v[i]=0;
        m_aux[i]=0;
    }
    m_aux[n-1]=0;
    for (j=0; j<n-1; j++){
        index=permutation[j];
        v[j]=index-m_aux[index];
        for (i=index; i<n; i++)
            m_aux[i]++;
    }
}
/*
 * Calculates the Kendall tau distance between 2 permutations.
 * Auxiliary parameters are used for multiple continuous executions.
 */
int Kendall(int* permutationA, int*permutationB, int size, int * m_aux, int * invertedB, int *  composition, int * v)
{
    int i,dist;
    dist=0;
    Invert(permutationB, size, invertedB);
    Compose(permutationA,invertedB,composition,size);
    vVector_Fast(v,composition,size,m_aux);
    for (i = 0; i < size-1; i++)
        dist += v[i];

    return dist;
}

float CalculateAvgKendallDistance(int ** samples_pool,int number_samples, int n, int*aux, int*inverted, int*composition, int*v){
    int sample_index, other_sample_index;
    int counter=0;
    float avg_distance=0;
    for (sample_index=0;sample_index<(number_samples-1);sample_index++){
        for (other_sample_index=sample_index+1;other_sample_index<number_samples;other_sample_index++){
           avg_distance+=Kendall(samples_pool[sample_index],samples_pool[other_sample_index],n, aux, inverted, composition, v);
           // Invert(samples_pool[other_sample_index], n, inverted);
           // Compose(samples_pool[sample_index],inverted,composition,n);
           // vVector_Fast(v,composition,n,aux);
           // for (i = 0; i < n-1; i++)
           // avg_distance += v[i];
            counter++;
        }
    }
    avg_distance=avg_distance/counter;

    return avg_distance;
}
