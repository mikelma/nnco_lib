/*
 *  PlackettLuce.cpp
 *  MixtureMallowsEDA
 *
 *  Created by Josu Ceberio Uribe on 7/16/12.
 *  Copyright 2012 University of the Basque Country. All rights reserved.
 *
 */

#include "PlackettLuce.h"
#include "Tools.h"
#include "LOP.h"
#include <math.h>
#include <complex>
//#define VERBOSE_PL 1

using namespace std;
ofstream fweights;
/*
 * Class constructor.
 */
CPlackettLuceModel::CPlackettLuceModel(int problem_size, int sel_size)
{
	m_problem_size=problem_size;
    m_samples_size=sel_size;
    
    m_MM_iterations=5000;
    m_thresholdMM=0.000001;
	m_weights= new double[m_problem_size];
    
    m_aux1= new int[m_problem_size];
    m_aux2= new int[m_problem_size];
    m_sampling_permutation= new int[m_problem_size];

    //creating sampling structures.
    distribution= new double[m_problem_size];
    objects= new int[m_problem_size];
    m_last= new int[m_problem_size];

        //create learning structures.
        m_M = m_problem_size;
        m_N = m_samples_size; //number of rankings in the population.
        m_P = m_problem_size;
    
        r2 = new double*[m_M];
        f= new int*[m_P];
        r= new int*[m_M];
        aux= new double*[m_P];
        g= new double*[m_P];
        for (int i=0;i<m_P;i++)
        {
            f[i]= new int[m_N];
            r[i]= new int[m_N];
            r2[i]= new double[m_N];
            aux[i]= new double[m_N];
            g[i]= new double[m_N];
        }
        
        dgamma= new double[m_M];
        gamma= new double[m_M];
        newgamma= new double[m_M];
    
        w= new double[m_M];
        pp= new int[m_N];
        sr2= new double[m_M];
   
    m_p1=new int[m_problem_size];
    m_p2= new int[m_problem_size];
    m_located= new int[m_problem_size];
}

/*
 * Class destructor.
 */
CPlackettLuceModel::~CPlackettLuceModel()
{
    delete [] m_sampling_permutation;
	delete [] m_weights;
    
    delete [] m_last;
    delete [] distribution;
    delete [] objects;
    delete [] m_aux1;
    delete [] m_aux2;
    
        for (int i=0;i<m_P;i++)
        {
            delete [] f[i];
            delete [] r[i];
            delete [] r2[i];
            delete [] aux[i];
            delete [] g[i];
        }
        delete [] f;
        delete [] r;
        delete [] r2;
        delete [] aux;
        delete [] g;
    
        delete [] dgamma;
        delete [] gamma;
        delete [] newgamma;
        delete [] w;
        delete [] pp;
        delete [] sr2;
    
    delete [] m_p1;
    delete [] m_p2;
    delete [] m_located;
}


/*
* It estimates the Plackett-Luce model from the given cases. The learning process consists
* of finding the MLE w parameters. For that task, Minorise-Maximize (MM) algorithm is utilized.
*/
bool CPlackettLuceModel::Learn(CPopulation * population, int size)
{
    //This code is a parse of the code of Hunter et al. 2004 of the MM algorithm implemented in matlab.
        
        //Initialization of auxiliary data.
        int i,j, aux_i,aux_j;
    //    int M = ProblemSize;
    //    int N = sel_total; //number of rankings in the population.
    //    int P = ProblemSize;

        int * individua;
        //extract individuals from population
        for (i = 0; i < m_N-1; i++)
        {
            individua = population->m_individuals[i]->Genes();
            for (j = 0; j < m_M; j++)
            {
                //we consider that individuals in the population have an ordering description
                //and these matrix consider ordering description of the solutions.
                f[j][i]=individua[j];
                r[individua[j]][i]= j + m_P*i;
            }
        }
        
        
        GenerateRandomPermutation( m_last,m_problem_size);
        for (j = 0; j < m_M; j++)
        {
            //we consider that individuals in the population have an ordering description
            //and these matrix consider ordering description of the solutions.
            f[j][i]=m_last[j];
            r[m_last[j]][i]= j + m_P*i;
        }
     
        //create a copy of r in r2.
        for (i=0;i<m_M;i++) for(j=0;j<m_N;j++)r2[i][j]=r[i][j];

        
        for (i=0;i<m_M;i++) w[i]=0;
        for (i=0;i<m_N;i++) pp[i]=m_M;

        for (i=0;i<m_N;i++)
            for (j=0;j<m_P-1;j++)
                w[f[j][i]]++;
            
        
        for (i=0;i<m_N;i++) pp[i]+= i*m_P;
        
        //Starts the MM algorithm

        for (i=0;i<m_M;i++) {gamma[i]=1;dgamma[i]=1;}

        int iterations=0;
        //double ** aux= new double*[P];
        m_norm=1;

        //cout<<"Starting MM algorithm...."<<endl;
        while(m_norm>m_thresholdMM && iterations<m_MM_iterations)
        {
            #ifdef VERBOSE_PL
                cout<<"norm: "<<m_norm<<"iterations: "<<iterations<<endl;
            #endif
            //PRIMERA PARTE
            iterations++;

            // for (i=0;i<N;i++) for(j=0;j<N;j++)g[i][j]=f[i][j]; //esta linea en matlab hace falta, pero aqui no.
     
            for(i=0;i<m_P;i++)
                for(j=0;j<m_N;j++)
                    g[i][j]=gamma[f[i][j]];
            
            //for (j=0;j<N;j++)
        //        aux[P-1][j]=g[P-1][j];
            
            memcpy(aux[m_P-1], g[m_P-1], sizeof(double)*m_N);
            
            for(i=m_P-2;i>=0;i--)
                for (j=0;j<m_N;j++)
                    aux[i][j]=g[i][j]+aux[i+1][j];

            for(i=m_P-1;i>=0;i--)
                memcpy(g[i],aux[i],sizeof(double)*m_N);
        
            
            for(j=0;j<m_N;j++)
                g[m_P-1][j]=0;
        
            
            for(i=0;i<m_M-1;i++)
                for(j=0;j<m_N;j++)
                    g[i][j]=(double)1/g[i][j]; //en este for anidado va a dar error porque la ultima fila de g es de zeros
            //como debe, y no estamos seguros d que se controle. Hay que ver si el N-1 esta bien puesto en el primer
            //for
            
            cumsum(g,m_P,m_N,g);

            //SEGUNDA PARTE
            for (j=0;j<m_N;j++)
            {
                for(i=0;i<m_M;i++)
                {
                    aux_i= r[i][j] % m_P;
                    aux_j= r[i][j] / m_P;
                    r2[i][j]=g[aux_i][aux_j];
                }
            }
            
            for (i=0;i<m_M;i++)
            {
                sr2[i] = sum(r2[i],m_N);
                newgamma[i] = w[i] / sr2[i];
                dgamma[i] = newgamma[i] - gamma[i];
            }

            memcpy(gamma, newgamma, sizeof(double)*m_M);

            m_norm=norm(dgamma, m_M);
            iterations++;
        }

        
        double sumGamma=sum(gamma,m_M);

        for (int i=0;i<m_M;i++) m_weights[i]=gamma[i]/sumGamma;
        //PrintArray(m_weights, m_M, "weights. ");
        
    return true;
}

/*
 * Samples the plackett-luce model learnt.
 */
long long int CPlackettLuceModel::Sample(CPopulation * population, long long int num_samples, LOP * problem)
{
	int i,j,k , remaining,pos, samples;
	double randVal,acumul,acumul_aux;
    
    for (samples=0;samples<num_samples ;samples++){
        
        for (i=0;i<m_problem_size;i++) { m_aux1[i]=-1;m_located[i]=0;}
        for (i=0;i<m_problem_size;i++)
        {

            //get ready data
            remaining=m_problem_size-i;
            acumul_aux=0;
            pos=0;
            for (j=0;j<m_problem_size;j++)
            {
                if (m_located[j]==0)//permutation[j]==-1)
                {
                    distribution[pos]=m_weights[j];
                    acumul_aux+=m_weights[j];
                    objects[pos]=j;
                    pos++;
                }
            }

            randVal=drand48()*acumul_aux;
            //randVal=(double)rand()/(((double)RAND_MAX/acumul_aux));
            acumul=distribution[0];
            for(k=1;(k<remaining && acumul<randVal);k++)
            {
                acumul += distribution[k];
            }
            k--;
        
            m_sampling_permutation[i]=objects[k];
            m_located[objects[k]]=1;
        }
        population->AddToPopulation(m_sampling_permutation, samples, problem->Evaluate(m_sampling_permutation));
    }
    return num_samples;
}

/*
 * Returns the probability of the 'indiv' individual for Plackett-Luce model.
 */
double CPlackettLuceModel::Probability(int * indiv)
{
    double probability=1;
    double aux,aux2;
    int i,j;

        for (i=0;i<m_problem_size;i++)
        {
            aux=m_weights[indiv[i]];
            aux2=0;
            for (j=i;j<m_problem_size;j++)
                aux2=aux2+m_weights[indiv[j]];
            probability=probability*(aux/aux2);
        }
    
	return probability;
}



