#ifndef RANDOM_H
#define RANDOM_H

#define SFMT_MEXP 19937

extern "C" {
	//SFMT 1.4.1 from http://www.math.sci.hiroshima-u.ac.jp/~m-mat/MT/SFMT/index.html#SFMT
	#include "SFMT.h"
}

#include <cstdio> //FILE


//state of the random number generator (declaration)
extern sfmt_t sfmt;


//NOTE: the vast majority of functions are inline


//init random number generator with seed
void initRand(unsigned int seed);


//get a random seed from /dev/random
unsigned int randSeed();


//random double in [0,1)
inline double urand() {
	return sfmt_genrand_real2(&sfmt);
}


//random double in [0,1]
inline double urandi() {
	return sfmt_genrand_real1(&sfmt);
}


//random integer in [0,b)
inline int irand(int b) {
	return sfmt_genrand_uint32(&sfmt)%b;
}


//two random indices with two calls to the rng
inline void twoRandIndices(int size, int& first, int& second) {
	first = irand(size);
	second = (first+1+irand(size-1))%size;
}


//first, second, third are integers in [0,size) different among them and with respect to diffFrom
void threeRandIndicesDiffFrom(int size, int diffFrom, int& first, int& second, int& third);


//perm is a random permutation of [0,size) obtained using Fisher-Yates shuffle
int* prand(int size, int* perm = 0);


//random bitstring
int* brand(int size, int* bits = 0);


//write rng state to a text file already opened without closing it
void writeRandState(FILE* f);


//read rng state from a text file already opened without closing it
void readRandState(FILE* f);


#endif

