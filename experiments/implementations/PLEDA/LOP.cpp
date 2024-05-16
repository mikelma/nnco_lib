//
//  LOP.cpp
//  VNSforLOP
//
//  Created by Josu Ceberio Uribe on 21/09/17.
//  Copyright © 2017 Collaboration Santucci - Ceberio. All rights reserved.
//

#include "LOP.h"
#include "Tools.h"
#include <algorithm>
#include <climits>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>

/*
 * Constructor method.
 */
LOP::LOP()
{
    m_problem_size=10;
    m_max_evaluations=0;
}

/*
 * Destructor method.
 */
LOP::~LOP()
{
    for (int i=0;i<m_problem_size;i++){
		delete [] m_instance[i];
    }
	delete [] m_instance;
}


/*
 * Read LOP instance from file.
 */
int LOP::Read(char * filename){
    
    // Open connection with file and read instance size.
    FILE *file=fopen(filename, "r");
    fscanf(file,"%d",&m_problem_size);
 #ifdef VERBOSE
    printf("%d\n", m_problem_size);
#endif
    // Initiaze structures.
    m_instance = new int*[m_problem_size];
    //cout<<"..... Allocating..."<<endl;
    for (int i=0;i<m_problem_size;i++)
    {
        m_instance[i]= new int[m_problem_size];
     }
    
    // Read instance.
    int size=100192;
    char line[size];
    char * chop;
    const char * delimiter=" ";
    int i=0, j=0;
    fgets(line, size, file);
    //Read the whole file to get the dimensions of data.
    while (fgets(line, size, file)!=NULL){
        if(line[0]!='\n'){
            chop=strtok(line,delimiter);
            j=0;
            while(chop!=NULL){
                m_instance[i][j]=atoi(chop);
                chop=strtok(NULL,delimiter);
                j++;
            }
        }
        i++;
    }
    fclose(file);
    
    m_evaluations=0;
    return m_problem_size;
}

/*
 * Calculates the corresponding objective value of the solution for the LOP problem.
 */
long LOP::Evaluate(int * solution)
{
	long value=0;
    int i,j;
    for (i=0;i<m_problem_size-1;i++){
		for (j=i+1;j<m_problem_size;j++){
			value+= m_instance[solution[i]][solution[j]];
        }
    }
    m_evaluations++;
	return value;
}

/*
 * Returns the size of the problem.
 */
int LOP::GetProblemSize()
{
    return m_problem_size;
}

/*
 * Returns the number of function evaluations remaining.
 */
long long LOP::GetRemaningBudget()
{
    return m_max_evaluations-m_evaluations;
}
