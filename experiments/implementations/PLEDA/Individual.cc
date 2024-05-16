//
//  Individual.cc
//  RankingEDAsCEC
//
//  Created by Josu Ceberio Uribe on 11/19/13.
//  Copyright (c) 2013 Josu Ceberio Uribe. All rights reserved.
//

#include "Individual.h"
#include "Tools.h"

// The object storing the values of all the individuals
// that have been created during the execution of the
// program. 

CIndividual::CIndividual(int length)
{
	m_size = length;
	m_genes = new int[m_size];
    m_value=MIN_LONG_INTEGER;
}

/*
 * The descructor of the class individual
 */
CIndividual::~CIndividual()
{
	delete [] m_genes;
	m_genes=NULL;
}

/*
 * Returns the fitness value of the solution.
 */
double CIndividual::Value()
{
	return m_value;
}

/*
 * Returns the genes of the individual.
 */
int * CIndividual::Genes()
{
	return m_genes;
}

/*
 * Output operator.
 */
ostream & operator<<(ostream & os,CIndividual * & individual)
{
    os <<individual->m_value<<"     ";
    os << individual->m_genes[0];
	for(int i=1;i<individual->m_size;i++)
        os << " " << individual->m_genes[i];
	return os;
}

/*
 * Input operator.
 */
istream & operator>>(istream & is,CIndividual * & individual)
{
  char k; //to avoid intermediate characters such as ,.

  is >> individual->m_value;
  is >> individual->m_genes[0];
  for(int i=1;i<individual->m_size;i++)
    is >> k >> individual->m_genes[i];
  is >> k;

  return is;
}

/*
 * Sets the given value as the new fitness of the individual.
 */
void CIndividual::SetValue(double value)
{
	m_value=value;
}

/*
 * Sets the given array of ints as the genes of the individual.
 */
void CIndividual::SetGenes(int * genes)
{
	memcpy(m_genes, genes, sizeof(int)*m_size);
	m_value=MIN_LONG_INTEGER;
}

/*
 * Prints in the standard output genes.
 */
void CIndividual::PrintGenes()
{
	for (int i=0;i<m_size;i++){
		cout<<m_genes[i]<<" ";
	}
	cout<<" "<<endl;
}

/*
 * Clones the individual.
 */
CIndividual * CIndividual::Clone()
{
	CIndividual * ind = new CIndividual(m_size);
	ind->SetGenes(m_genes);
	ind->SetValue(m_value);
	return ind;
}



