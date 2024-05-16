#ifndef LOP_INSTANCE_H
#define LOP_INSTANCE_H

#include <string>
using namespace std;

struct LopInstance {

	string filename;
	int* lop1 = 0;
	int n = 0;
	int opt,ub,lb;

	LopInstance(string filename);
	~LopInstance();

	int eval(int* x);

	void readInstance(string filename);
	void bounds();

};

#endif
