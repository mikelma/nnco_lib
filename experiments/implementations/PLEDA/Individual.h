//
//  Individual.h
//  RankingEDAsCEC
//
//  Created by Josu Ceberio Uribe on 11/19/13.
//  Copyright (c) 2013 Josu Ceberio Uribe. All rights reserved.
//

#ifndef _INDIVIDUAL_
#define _INDIVIDUAL_

#include <stdlib.h>
#include <stdio.h>
#include <fstream>
#include <limits.h>
#include <iostream>
#include "Variables.h"
using namespace std;
using std::istream;
using std::ostream;

class CIndividual
{
public:

	// The constructor. The constructed individual has
	// all zeroes as its genes.
	CIndividual(int length);

	// The destructor. It frees the memory allocated at
	// the construction of the individual.
	virtual ~CIndividual();

	/*
     * It returns an array with the genes of the individual.
     */
	int * Genes();

	/*
     * Returns the value of the individual.
     */
	double Value();
	
    /*
     * Sets the given value as the new fitness of the individual.
     */
	void SetValue(double value);
    
    /*
     * Sets the given array of ints as the genes of the individual.
     */
	void SetGenes(int * genes);
		
    /*
     * Prints in the standard output genes.
     */
	void PrintGenes();
	
    /*
     * Clones the individual.
     */
	CIndividual * Clone();

    /*
     * Output operator.
     */
	friend ostream & operator<<(ostream & os,CIndividual * & individual);
    
    /*
     * Input operator.
     */
	friend istream & operator>>(istream & is,CIndividual * & individual);

       
private:

	/*
     * The variable storing the individual's fitness value.
     */
	double m_value;

	 /*
      * The genes of the individual.
      */
	int * m_genes;
	
	/*
     * The size of the individual
     */
	int m_size;
};

#endif
