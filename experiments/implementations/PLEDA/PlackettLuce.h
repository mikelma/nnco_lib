/*
 *  PlackettLuce.h
 *  MixtureMallowsEDA
 *
 *  Created by Josu Ceberio Uribe on 7/16/12.
 *  Copyright 2012 University of the Basque Country. All rights reserved.
 *
 */


#ifndef PLACKETTLUCE_H__
#define PLACKETTLUCE_H__

#include <list>
#include <vector>
#include "Population.h"
#include "LOP.h"
class CPlackettLuceModel
{
public:
    
	/*
	 * The constructor.
	 */
	CPlackettLuceModel(int problem_size, int sel_size);
	
    /*
	 * The destructor.
	 */
    virtual ~CPlackettLuceModel();
	
    /*
     * Given a population of samples, it learns a Plackett-Luce model from the data.
     */
    bool Learn(CPopulation * population, int size);
    
	/*
	 * Simulates a new individual sampling the probabilistic model.
	 */
    long long int Sample(CPopulation * population, long long int num_samples, LOP * problem);
    
	/*
     * Calculates ThetaParameters average value.
     */
	double GetWeightsAverage();
	
	/*
	 * Calculates the probability of the individual given the probabilistic model.
	 */
	double Probability(int * individual);
    
    
private:
    
    /*
	 * It estimates the Plackett-Luce model from the given cases with the MM algorithm.
	 */
	bool LearnMM(CPopulation * population, int sel_total);
    
    
    CIndividual * SimulateInverse();
    
    /*
     * Calculates the most probable solution given the vector of weights
     */
    void CalculateMostProbableSolution();

    /*
     * Returns the most probable solution given the vector of weights
     */
    CIndividual * GetMostProbableSolution();
    	
	/*
     * Problem size.
     */
    int m_problem_size;

    /*
     * Number of samples.
     */
    int m_samples_size;
    
    /*
     * Number of maximum iterations to perform by the MM algorithm.
     */
    int m_MM_iterations;

    /*
     * The threshold for MM.
     */
    double m_thresholdMM;
    
	/*
     * Parameter vector v
     */
	double * m_weights;
    	
    /* 
     * Euclidean norm of the weights.
     */
    double m_norm;
   
    
    /*
     * The auxiliary vector for sampling solutions.
     */
    int * m_sampling_permutation;

    /*
     * Auxiliary data.
     */
    int * m_aux1;
    int * m_aux2;
    
    // Number of items fixed in the template.
    int m_k;
    
    //Parameters for learning
    int m_M;
	int m_N; //number of rankings in the population.
	int m_P;
    int ** f;
    int ** r;
    double ** r2;
    double ** g;
    double * dgamma;
	double * gamma;
    double * w;
	int * pp;
    double **aux;
    double * newgamma;
    double * sr2;
    int * m_last;
    int m_estimation_type;
    
    //parameters for sampling
    double * distribution;
	int * objects;
    
    //forward-backward sampling arrays
    int * m_located;
    int * m_p1;
    int * m_p2;

};
#endif
