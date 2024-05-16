/*
 *  Tools.h
 *  RankingEDAsCEC
 *
 *  Created by Josu Ceberio Uribe on 11/21/11.
 *  Copyright 2011 University of the Basque Country. All rights reserved.
 *
 */
#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string.h>
#include <stdio.h>
#include <vector>
#include <math.h>
#include <complex>
using namespace std;
using std::istream;
using std::ostream;

using std::cerr;
using std::cout;
using std::endl;
using std::ifstream;
using std::stringstream;
using std::string;

double norm(double * values, int size);
double sum (double * values, int size);
void cumsum(double ** values,int size1, int size2, double ** resul);
void cumsum(double * values, int size, double * resul);

/*
 * Returs the first position at which value appears in array. If it does not appear, then it returns -1;
 */
int Find(int * array, int size, int value);

/*
 * Calculates the average value of the parameters in the array.
 */
double Average(double * params, int size);

double Variance(double * params, int size, double avg );

/*
 * It determines if the given int sequecen if it is indeed a permutation or not.
 */
bool isPermutation(int * permutation, int size);

/*
 * Generates a random permutation of size 'n' in the given array.
 */
void GenerateRandomPermutation(int * permutation, int n);

/*
 * Determines if a given string contains a certain substring.
 */
bool strContains(const string inputStr, const string searchStr);

/*
 * Prints in standard output 'length' integer elements of a given array.
 */
void PrintArray(int* array, int length, string text);

/*
 * Prints in standard output 'length' long double elements of a given array.
 */
void PrintArray(long double* array, int length, string text);

/*
 * Prints the given doubles array in the standard output.
 */
void PrintArray(double* array, int length, string text);

/*
 * Prints the given integers matrix in the standard output.
 */
void PrintMatrix(int** matrix, int length, int length2, string text);

/*
 * Prints the given doubles matrix in the standard output.
 */
void PrintMatrix(double** matrix, int length, int length2, string text);

/*
 * Applies the random keys sorting strategy to the vector of doubles
 */
void RandomKeys( int * a, double * criteriaValues, int size);

/*
 * Calculates the factorial of a solution.
 */
long double factorial(int val);

