//
//  main.cpp
//  PlackettLuceEDA
//
//  Created by JosuC on 18/02/2020.
//  Copyright © 2020 JosuC. All rights reserved.
//

#include <iostream>
#include "ParameterSetting.h"
#include "PLEDA.hpp"
#include "Individual.h"
#include "LOP.h"
#include <iostream>
#include <sys/time.h>
#include "random.h"
#include "unistd.h"


int main(int argc, char * argv[])
{    
    //0. Get parameters from command line
    if(!GetParameters(argc,argv))
        return -1;
        
    #ifdef VERBOSE
        cout<<"Reading instance..."<<endl;
    #endif
    
    //1. Read LOP instance, and initialize max_evaluations
    LOP * lop= new LOP();
    cout<<"Instance: "<<INSTANCE_FILENAME<<endl;
    cout<<"Seed: "<<SEED<<endl;
    int n=lop->Read(INSTANCE_FILENAME);
    lop->m_max_evaluations=(long long int)1000*n*n;
     
    //2. Start timing and set seed
    struct timeval tim;
    gettimeofday(&tim, NULL);
    double start_time=tim.tv_sec+(tim.tv_usec/1000000.0);
    double current_time;
        
    #ifdef VERBOSE
        cout<<"Stopping criteria..."<<endl;
        cout<<"Evals: "<<lop->m_max_evaluations<<endl;
    #endif
    initRand(SEED);
    
    //3. Create algorithms structure
    PLEDA * algorithm= new PLEDA(lop);
    
    //4. Launch PLEDA
    algorithm->Run();
    CIndividual* best = algorithm->GetBestSolution();
    
    //5. Calculate consumed time.
     gettimeofday(&tim, NULL);
     current_time=tim.tv_sec+(tim.tv_usec/1000000.0);
     
     //6. Write results in file.
     FILE *result_file;
    #ifndef RUNNING_ON_CLUSTER
        FILE * test;
        test= fopen("/Users/JosuC/Desktop/results.csv","r");
        result_file= fopen("/Users/JosuC/Desktop/results.csv","a+");
        if (test==NULL){
            fprintf(result_file,"\"Instance\";\"Repetition\";\"Algorithm\";\"Fitness\";\"Real_Time\"\n");
        }
    fprintf(result_file,"\"%s\";%d;\"%s\";%.3f;%.3f\n",INSTANCE_FILENAME,SEED,"PLEDA",best->Value(),current_time-start_time);
    #else
        srand(getpid());
        string fname = "results/" + std::to_string(rand()) + ".csv";
        result_file= fopen(fname.c_str(),"a+");
    fprintf(result_file,"\"%s\";%d;\"%s\";%.3f;%.3f\n",INSTANCE_FILENAME,SEED,"PLEDA",best->Value(),current_time-start_time);
    #endif
        fclose(result_file);
    return 0;
}
