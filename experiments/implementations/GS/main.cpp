#include "utils.h"
#include "random.h"
#include "LopInstance.h"
#include "algorithms.h"
#include "BradleyTerry.h"
#include <cstdlib>
#include <cstring>
#include <string>
#include <iostream>
#include <sys/time.h>
using namespace std;

#include "Variables.h"


int main(int argc, char** argv) {

	//some variables and their default values
	string lopInstanceName;
    string model="PL";

    ldouble alpha = 0.01L;
	int lambda = 100;
    int lowerLambda=10;
    int upperLambda=1000;
    ldouble lowerAlpha=1e-4;
    ldouble upperAlpha=0.9;

	int maxEvaluations = 10000;
    int sampling_iterations=40; //this is for the BT, and still to tune this value.
	unsigned int seed = 0;
	int printInterval = 100;
	string utilityFunction = "superlinear";
	//read command line arguments
	if (argc<2) {
		cerr << "USAGE: ./gradientSearch LOP_INSTANCE_FILE RUNNING_MODE [MODEL=PL] [SEED=random] [LAMDA=100] [ALPHA=0.01] [PRINT_INTERVAL=5] \n";
        cerr << "       Utility functions available: fitness|normalizedFitness|superlinear|linear|equal\n";
        cerr << "       Models available: PL|PLN|PLSL|PLSS|PLSA|PLNSA!BT\n";
		return EXIT_FAILURE;
	}

	lopInstanceName.assign(argv[1],strlen(argv[1]));
    int RUNNING_MODE=atoi(argv[2]);
    if (argc>3) model=model.assign(argv[3],strlen(argv[3]));
	if (argc>4) sscanf(argv[4],"%u",&seed);
    if (argc>5) lambda = atoi(argv[5]);
    if (argc>6) alpha = atof(argv[6]);
    if (argc>7) printInterval = atoi(argv[7]);

    srand(seed);
	//load LOP instance
	LopInstance lop(lopInstanceName);
    //lop.n=5;
    maxEvaluations=1000*lop.n*lop.n;

	//run gradient ascent
    struct timeval tim;
    gettimeofday(&tim, NULL);
    double start_time=tim.tv_sec+(tim.tv_usec/1000000.0);
    int* best;
    if (model=="PL"){ //standard model without self adaptation
        /*
        if (RUNNING_MODE==0 || RUNNING_MODE==2){
            if (lop.n==150 || lop.n==250){
                //xLOLIB
	// if(lop.n==150){
	                alpha=0.05; //in the original paper there is confusion, but I think we used 0.1, that is why the results now are a bit worse.
        	        lambda=100;
		//}
		//else{
		//	alpha=0.1;
		//	lambda=1000;
		//}
            }
            else{
                //IO benchmark
                alpha=0.9;
                lambda=100;
            }
        }
        */
        best=gradientAscent_PL(lopInstanceName,lop, RUNNING_MODE, alpha,lambda,maxEvaluations,utilityFunction,printInterval,seed, lowerAlpha, upperAlpha); //it prints on the standard output because verbose=true
    }
    else if (model=="PLN"){ //natural gradients with model without self adaptation
        if (RUNNING_MODE==0 || RUNNING_MODE==2){
            if (lop.n==150 || lop.n==250){
                //xLOLIB
//		if (lop.n==150){
                	alpha=0.5;
                	lambda=10;
//		}
//		else{
//			alpha=0.5;
//			lambda=10;
//		}
            }
            else{
                //IO benchmark
                alpha=0.9;
                lambda=100;
            }
        }
        best=naturalgradientAscent_PL(lopInstanceName,lop,RUNNING_MODE, alpha,lambda,maxEvaluations,utilityFunction,printInterval,seed,lowerAlpha, upperAlpha); //it prints on the standard output because verbose=true
    }
    else if (model=="PLNSA"){ //self adapting sampling for natural gradients
        if (lop.n==150 || lop.n==250){
            //xLOLIB
            if (lop.n==150){
                alpha=0.5; //in the original paper there is confusion, but I think we used 0.1, that is why the results now are a bit worse.
                lambda=10;
            }
            else{
                alpha=0.5;
                lambda=10;
            }
        }
        else{
            //IO benchmark
            alpha=0.75;
            lambda=100;
        }
        best=naturalgradientAscent_PL_selfadjusting_total(lopInstanceName,lop,RUNNING_MODE,lambda, lowerLambda, upperLambda, alpha, maxEvaluations,utilityFunction,printInterval,seed,lowerAlpha, upperAlpha); //it prints on the standard output because verbose=true
    }
    else if (model=="PLSA"){ //self adapting learning and sampling
        if (lop.n==150 || lop.n==250){
            //xLOLIB
            if(lop.n==150){
                alpha=0.05;
                lambda=100;
            }
            else{
                alpha=0.1;
                lambda=1000;
            }
        }
        else{
            //IO benchmark
            alpha=0.5;
            lambda=100;
        }
        best=gradientAscent_PL_selfadjusting_total(lopInstanceName,lop,RUNNING_MODE, lambda, lowerLambda, upperLambda, alpha, maxEvaluations,utilityFunction,printInterval,seed,lowerAlpha, upperAlpha); //it prints on the standard output because verbose=true
    }
    else //if (model="BT")
    {
       // best=gradientAscent_BT(lopInstanceName,lop,RUNNING_MODE,alpha,lambda,sampling_iterations,maxEvaluations,utilityFunction,printInterval,seed);
    }
    gettimeofday(&tim, NULL);
    double end_time=tim.tv_sec+(tim.tv_usec/1000000.0);
    //done
    if (RUNNING_MODE==0 || RUNNING_MODE==1){
    #ifndef RUNNING_ON_PRISCILLA
    //Print in the standard output
        cout<<lopInstanceName<<";"<<seed<<";"<<model<<";"<<lambda<<";"<<alpha<<";"<<best[0]<<";"<<best[1]<<";"<<end_time-start_time<<endl;
    #else
    //Print in scratch_results.csv file
    FILE *result_file;
    char fname[30];
    snprintf(fname, 30, "results/%d.csv", seed);
    result_file= fopen(fname,"a+");
    fprintf(result_file,"instance,seed,algorithm,lambda,alpha,best fitness,best evals,elapsed time in secs\n",lopInstanceName.c_str(),seed,model.c_str(),lambda,alpha,best[0],best[1],end_time-start_time);
    fprintf(result_file,"\"%s\",%d,\"%s\",%d,%.3Lf,%d,%d,%.3f\n",lopInstanceName.c_str(),seed,"GS",lambda,alpha,best[0],best[1],end_time-start_time);
    fclose(result_file);
    #endif
    }
	return EXIT_SUCCESS;
}
