//
//  LOP.h
//  VNSforLOP
//
//  Created by Josu Ceberio Uribe on 21/09/17.
//  Copyright © 2017 Collaboration Santucci - Ceberio. All rights reserved.
//

#ifndef _LOP_H__
#define _LOP_H__

#include <stdlib.h>
#include <math.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <stdio.h>


using std::istream;
using std::ostream;
using namespace std;
using std::cerr;
using std::cout;
using std::endl;
using std::ifstream;
using std::stringstream;
using std::string;

class LOP 
{
	
public:

    /*
     * Matrix of parameters of the instance.
     */
    int ** m_instance;

    /*
     * The size of the problem.
     */
    int m_problem_size;
    
    /*
     * Maximum number of evaluations allowed to perform.
     */
    long long int m_max_evaluations;
        
    /*
     * The number of evaluations currently performed.
     */
    long long int m_evaluations;
    
    /*
     * Constructor method.
     */
	LOP();
   
    /*
     * The destructor.
     */
    virtual ~LOP();
	
	/*
	 * Read LOP instance from file.
	 */
    int Read(char * filename);
    
	/*
	 * Calculates the corresponding objective value of the solution for the LOP problem.
	 */
	long Evaluate(int * solution);
	
    
    /*
     * Returns the size of the problem.
     */
    int GetProblemSize();

    /*
     * Returns the number of function evaluations remaining.
     */
    long long int GetRemaningBudget();
private:


    
    
};
#endif





