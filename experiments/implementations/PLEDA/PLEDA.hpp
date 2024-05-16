//
//  PLEDA.hpp
//  PlackettLuceEDA
//
//  Created by JosuC on 18/02/2020.
//  Copyright © 2020 JosuC. All rights reserved.
//

#ifndef PLEDA_hpp
#define PLEDA_hpp

#include <stdio.h>
#include <math.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string.h>
#include <stdio.h>
#include "LOP.h"
#include "Tools.h"
#include "Population.h"
#include "PlackettLuce.h"

using std::istream;
using std::ostream;
using namespace std;
using std::cerr;
using std::cout;
using std::endl;
using std::ifstream;
using std::stringstream;
using std::string;

class PLEDA
{
    
public:
    
    /*
     * Problem
     */
    LOP * m_problem;
    
    /*
     * Problem size
     */
    int m_problem_size;
    
    /*
     * The best solution found so far.
     */
    CIndividual * m_best;
    
    /*
     * The size of the population.
     */
    int m_pop_size;
    
    /*
     * The size of the selection pool.
     */
    int m_sel_size;
    
    /*
     * The size of the offspring population.
     */
    int m_offspring_size;

    /*
     * The population
     */
    CPopulation * m_population;
    
    /*
     * The probalistic model.
     */
    CPlackettLuceModel * m_model;
    
    /*
     * The constructor.
     */
    PLEDA(LOP * problem);
    
    /*
     * The destructor.
     */
    virtual ~PLEDA();
    
    /*
     * Running function
     */
    int Run();
    
    /*
     * Returns the fitness of the best solution obtained.
     */
    double GetBestSolutionFitness();
    
    /*
     * Returns the best solution obtained.
     */
    CIndividual * GetBestSolution();
    
private:
    
};
#endif /* PLEDA_hpp */
