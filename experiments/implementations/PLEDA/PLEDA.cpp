//
//  PLEDA.cpp
//  PlackettLuceEDA
//
//  Created by JosuC on 18/02/2020.
//  Copyright © 2020 JosuC. All rights reserved.
//

#include "PLEDA.hpp"
#include <iomanip>

//#define VERBOSE_1 1
//#define VERBOSE_2 1
/*
 * The constructor.
 */
PLEDA::PLEDA(LOP * problem)
{
    //1. standard initializations
    m_problem=problem;
    m_problem_size=problem->GetProblemSize();
    m_best= new CIndividual(m_problem_size);
    m_best->SetValue(MIN_LONG_INTEGER);
    m_pop_size=m_problem_size*10;
    m_sel_size=m_problem_size;
    m_offspring_size= m_pop_size-1;

    //m_upper_theta_bound=10;
    //2. initialize the population
    m_population= new CPopulation(m_pop_size, m_offspring_size,m_problem_size);
    int * genes= new int[m_problem_size];
    for(int i=0;i<m_pop_size;i++)
    {
        //Create random individual
        GenerateRandomPermutation(genes,m_problem_size);
        m_population->SetToPopulation(genes, i, m_problem->Evaluate(genes));
    }
    
    m_population->SortPopulation(0);
    delete [] genes;
    
    //3. Build model structures
    m_model= new CPlackettLuceModel(m_problem_size, m_sel_size);
#ifdef VERBOSE_1
    m_population->Print();
#endif
}

/*
 * The destructor. It frees the memory allocated..
 */
PLEDA::~PLEDA()
{
    delete m_best;
    delete m_population;
    delete m_model;
}


/*
 * Running function
 */
int PLEDA::Run(){
    
    //variable initializations.
    int iterations=1;

    //EDA iteration. Stopping criterion is the maximum number of evaluations performed
    while (m_problem->GetRemaningBudget()>0){
        
        //learn model
        #ifdef VERBOSE_2
         cout<<"learning..."<<endl;
        #endif
        m_model->Learn(m_population, m_sel_size);
        
        //sample de model
        #ifdef VERBOSE_2
        cout<<"sampling..."<<endl;
        #endif
        if (m_offspring_size<m_problem->GetRemaningBudget()){
            m_model->Sample(m_population,m_offspring_size, m_problem);
        }
        else{
            m_model->Sample(m_population,m_problem->GetRemaningBudget(), m_problem);
        }
        
        //update the model.
        #ifdef VERBOSE_2
        cout<<"sorting..."<<endl;
        #endif
        m_population->SortPopulation(1);

        //update indicators
        if (m_population->m_individuals[0]->Value()>m_best->Value())
        {
            #ifdef VERBOSE_1
            cout<<"Iteration: "<<iterations<<" Best: "<<m_population->m_individuals[0]->Value()<<" Remaining: "<<m_problem->GetRemaningBudget()<<endl;
            #endif
            m_best->SetGenes(m_population->m_individuals[0]->Genes());
            m_best->SetValue(m_population->m_individuals[0]->Value());
        }
        iterations++;
    }
    return 0;
}

/*
 * Returns the fitness of the best solution obtained.
 */
double PLEDA::GetBestSolutionFitness(){
    return m_best->Value();
}

/*
 * Returns the best solution obtained.
 */
CIndividual * PLEDA::GetBestSolution(){
    return m_best;
}

