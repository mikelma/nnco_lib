/*
 *  Tools.cpp
 *  RankingEDAsCEC
 *
 *  Created by Josu Ceberio Uribe on 11/21/11.
 *  Copyright 2011 University of the Basque Country. All rights reserved.
 *
 */

#include "Tools.h"
#include <math.h>
#include <sys/time.h>
#include "Variables.h"



void cumsum(double * values, int size, double * resul)
{
    resul[0]=values[0];
    for (int i=1;i<size;i++)
    {
        resul[i]=resul[i-1]+values[i];
    }
}
void cumsum(double ** values,int size1, int size2, double ** resul)
{
    for (int j=0;j<size2;j++)
        resul[0][j]=values[0][j];
    
    for(int i=1;i<size1;i++)
        for (int j=0;j<size2;j++)
            resul[i][j]=values[i][j]+resul[i-1][j];
}

double sum (double * values, int size)
{
    double res=0;
    for (int i=0;i<size;i++) { res+=values[i];}
    return res;
}
double norm(double * values, int size)
{
    double result=0;
    for (int i=0;i<size;i++)
        result+=pow(values[i],2);
    return sqrt(result);
}

/*
 * Returs the first position at which value appears in array. If it does not appear, then it returns -1;
 */
int Find(int * array, int size, int value){
    int i=0;
    while (i<size){
        if (array[i]==value)
            return i;
        i++;
    }
    return -1;
}
/*
 * Calculates the average value of the parameters in the array.
 */
double Average(double * params, int size){
    double acum=0;
    for (int i=0;i<size;i++){
        acum+=params[i];
    }
    return acum/size;
}


double Variance(double * params, int size, double avg )
{
    if (avg==NULL)
        avg = Average(params,size);
    
    double temp = 0;
    for(int i = 0; i < size; i++)
    {
        temp += (params[i] - avg) * (params[i] - avg) ;
    }
    return temp / size;
}

/*
 * It determines if the given int sequecen if it is indeed a permutation or not.
 */
bool isPermutation(int * permutation, int size)
{
	int flags[size];
	//int * flags=new int[size];
	for (int i=0;i<size;i++) flags[i]=1;
	
	for (int i=0;i<size;i++)
	{
		int value=permutation[i];
		flags[value]=0;
	}
	
	int result,sum=0;
	for(int i=0;i<size;i++)
		sum+=flags[i];
	if (sum==0) result=true;
	else result=false;
	//delete [] flags;
	return result;
}

/*
 * Generates a random permutation of size 'n' in the given array.
 */
void GenerateRandomPermutation(int * permutation, int n)
{
	for (int i = 0; i < n; ++i)
	{
		int j = rand() % (i + 1);
		permutation[i] = permutation[j];
		permutation[j] = i;
	}
}

/*
 * Determines if a given string contains a certain substring.
 */
bool strContains(const string inputStr, const string searchStr)
{
	size_t contains;
	
	contains = inputStr.find(searchStr);
	
	if(contains != string::npos)
		return true;
	else
		return false;
}

/*
 * Prints in standard output 'length' integer elements of a given array.
 */
void PrintArray(int* array, int length, string text)
{
	cout<<text;
	for (int i=0;i<length;i++){
		cout<<array[i]<<" ";
	}
	cout<<" "<<endl;
}


/*
 * Prints in standard output 'length' long double elements of a given array.
 */
void PrintArray(long double* array, int length, string text)
{
	cout<<text;
	for (int i=0;i<length;i++){
		cout<<array[i]<<" ";
	}
	cout<<" "<<endl;
}

/*
 * Prints in the standard output the 'size' elements of the given array..
 */
void PrintArray(int * array, int size)
{
	for (int i=0;i<size;i++){
		cout<<array[i]<<" ";
	}
	cout<<" "<<endl;
}



/*
 * Prints in standard output 'length' double elements of a given array.
 */
void PrintArray(double* array, int length, string text)
{
    int i;
	cout<<text;
	for (i=0;i<length;i++)
		printf("%3.5f,",array[i]);
	printf("\n ");
}

/*
 * Prints in the standard output given matrix.
 */
void PrintMatrix(int** matrix, int length, int length2, string text)
{
    int i,j;
	cout<<text;
	for (i=0;i<length;i++)
	{
		cout<<""<<endl;
		for (j=0;j<length2;j++)
		{
			cout<<matrix[i][j]<<" ";
		}
	}
	cout<<" "<<endl;
}

void PrintMatrix(double** matrix, int length, int length2, string text)
{
    int i,j;
	cout<<text;
	for (i=0;i<length;i++)
	{
		cout<<""<<endl;
		for (j=0;j<length2;j++)
		{
			cout<<matrix[i][j]<<", ";
		}
	}
	cout<<" "<<endl;
}



/*
 * Prints in standard output 'lengthxlength' double elements of a given matrix.
 */
void PrintMatrixDouble(double** matrix, int length, int length2, string text)
{
    int i,j;
	cout<<text<<endl;
	for (i=0;i<length;i++)
	{
		for (j=0;j<length2;j++)
			printf("%3.9f, ",matrix[i][j]);
		printf("\n");
	}
}
