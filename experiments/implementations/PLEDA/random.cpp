#include "random.h"
#include <cstdio> //FILE, fopen(), fclose(), fread(), fprintf(), fscanf()


//state of the random number generator (definition, declared in random.h)
sfmt_t sfmt;


//init random number generator with seed
void initRand(unsigned int seed) {
	sfmt_init_gen_rand(&sfmt,seed);
}


//get a random seed from /dev/random
unsigned int randSeed() {
	//this works only on linux (it uses /dev/random)
	FILE* f = fopen("/dev/urandom","rb");
	if (!f)
		return 0;
	unsigned int s;
	int nowarning = fread(&s,sizeof(unsigned int),1,f);   //nowarning to avoid compiler complaint
	nowarning++;                                          //to avoid compiler complaint
	fclose(f);
	return s;
}


//first, second, third are integers in [0,size) different among them and with respect to diffFrom
void threeRandIndicesDiffFrom(int size, int diffFrom, int& first, int& second, int& third) {
	//3 calls to the rng
	first = ( diffFrom + 1 + irand(size-1) ) % size; //first in [0,size[ excluded diffFrom
	int min,med,max;
	if (first<diffFrom) {
		min = first;
		max = diffFrom;
	} else {
		min = diffFrom;
		max = first;
	}
	second = ( min + 1 + irand(size-2) ) % size;
	if (second>=max || second<min)
		second = (second+1)%size;
	if (second<min) {
		med = min;
		min = second;
	} else if (second>max) {
		med = max;
		max = second;
	} else
		med = second;
	third = ( min + 1 + irand(size-3) ) % size;
	if (third<min || third>=max-1)
		third = (third+2)%size;
	else if (third>=med)
		third++; //no modulo since I'm sure to not overflow size
}


//perm is a random permutation of [0,size) obtained using Fisher-Yates shuffle
int* prand(long int size, int* perm) {
	//if memory not provided, allocate
	if (!perm)
		perm = new int[size];
	//Fisher-Yates shuffle using "inside-out" implementation (see Wikipedia page for Fisher-Yates)
	int j;
	perm[0] = 0;
	for (int i=1; i<size; i++) {
		j = irand(i+1); //j in [0,i]
		if (j!=i)
			perm[i] = perm[j];
		perm[j] = i;
	}
	//return pointer to the permutation
	return perm;
}


//write rng state to a text file already opened without closing it
void writeRandState(FILE* f) {
	//write sfmt.state
	int i,j;
	for (i=0; i<SFMT_N; i++) //sfmt.state has length SFMT_N
		for (j=0; j<4; j++) //sfmt.state[i].u has length 4
			fprintf(f,"%u ",sfmt.state[i].u[j]); //%u since it is uint32_t
	//write sfmt.idx
	fprintf(f,"%d\n",sfmt.idx); //%d since it is int
	//done
}


//read rng state from a text file already opened without closing it
void readRandState(FILE* f) {
	//read sfmt.state
	int i,j,nowarning; //nowarning is to avoid compiler complaints
	for (i=0; i<SFMT_N; i++) //sfmt.state has length SFMT_N
		for (j=0; j<4; j++) //sfmt.state[i].u has length 4
			nowarning = fscanf(f,"%u",&(sfmt.state[i].u[j])); //%u since it is uint32_t
	//read sfmt.idx
	nowarning = fscanf(f,"%d",&(sfmt.idx)); //%d since it is int
	//avoid compiler complaints
	nowarning++;
	//done
}

