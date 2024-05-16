#include "LopInstance.h"
#include <cstring>
#include <cstdio>
#include <climits>
#include <cstdlib>
#include <iostream>
#include <string>
using namespace std;

#define STR_SIZE 512


LopInstance::LopInstance(string filename) {
	readInstance(filename);
	bounds();
    //n=5; //remove this!
}


LopInstance::~LopInstance() {
	if (lop1) delete[] lop1;
}


void LopInstance::readInstance(string filename) {
	this->filename = filename;
	FILE* f = fopen(filename.c_str(),"r");
	if (!f) {
		cerr << "ERROR: Unable to open " << filename << endl;
		exit(EXIT_FAILURE);
	}
	bool dotmat = false;
	if (strstr(filename.c_str(),".mat")) dotmat = true;
	if (dotmat) while (fgetc(f)!='\n');	//jump the first line
	if (fscanf(f,"%d",&n)!=1) {
		cerr << "ERROR: Unable to read n from " << filename << endl;
		exit(EXIT_FAILURE);
	}
	int n2 = n*n;
	if (!lop1) lop1 = new int[n2];
	for (int i=0; i<n2; i++)
		if (fscanf(f,"%d",&lop1[i])!=1) {
			cerr << "ERROR: Unable to read " << i << "-th (0-based) entry from " << filename << endl;
			exit(EXIT_FAILURE);
		}
	if (fscanf(f,"%d",&opt)!=1)
		opt = -1;
	fclose(f);
	if (dotmat) {
		char optfile[STR_SIZE];
		int j;
		for (j=0; filename[j]!='.'; j++) optfile[j] = filename[j];
		optfile[j] = '.';
		optfile[j+1] = 'o';
		optfile[j+2] = 'p';
		optfile[j+3] = 't';
		optfile[j+4] = '\0';
		f = fopen(optfile,"r");
		if (f) {
			char line[STR_SIZE];
			if (!fgets(line,STR_SIZE,f)) {
				cerr << "ERROR: Unable to read optimum from " << optfile << endl;
				exit(EXIT_FAILURE);
			}
			fclose(f);
			if (!strstr(line,"optimum solution")) {
				//line has this structure:   Value               : 3095130
				char s1[STR_SIZE],semicolon;
				if (sscanf(line,"%s %c %d",s1,&semicolon,&opt)!=3) {
					cerr << "ERROR: Unable to read optimum (2) from " << optfile << endl;
					exit(EXIT_FAILURE);
				}
			} else {
				//line has this structure: BELGIUM T59B11XXB, optimum solution (value 245750)
				char s2[STR_SIZE];
				int k;
				for (j=0; line[j]!='('; j++);
				for (k=j+1; line[k]!=')'; k++)
					s2[k-j-1] = line[k];
				s2[k-j-1] = '\0';
				if (sscanf(s2,"%s %d",line,&opt)!=2) {
					cerr << "ERROR: Unable to read optimum (3) from " << optfile << endl;
					exit(EXIT_FAILURE);
				}
			}
		}
		fclose(f);
	}
	//done
}


int LopInstance::eval(int* x) {
	int i,j,xin;
	int r = 0;
	for (i=0; i<n; i++) {
		xin = x[i]*n;
		for (j=i+1; j<n; j++)
			r += lop1[xin+x[j]];
	}
	return r;
}


void LopInstance::bounds() {
	int i,j,min,max;
	lb = ub = 0;
	for (i=0; i<n; i++) {
		for (j=i+1; j<n; j++) {
			if (lop1[i*n+j]>lop1[j*n+i]) {
				min = lop1[j*n+i];
				max = lop1[i*n+j];
			} else {
				min = lop1[i*n+j];
				max = lop1[j*n+i];
			}
			lb += min;
			ub += max;
		}
	}
}
