#include <iostream>
#include <fstream>
#include "mpi.h"
#include <cstdio>
#include <cstring>
#include <stdlib.h>
#include <vector>
#define ROOT 0
using namespace std;


//Global to print out 
int VERBOSE = false;


int main (int argc, char *argv[])
{

//Mpi variables
int actorCount;
int echelon;

//Time data
double start;
double end;

//Matrix data
int N;
int **matA;
int **matB;
int **matC;

/*
//File data
ifstream fileA;
ifstream fileB;
fileA.open(argv[1]);
fileB.open(argv[2]);
*/
//Read in the first number, contains matrix dim
//fileA >> N;
//fileB >> N;
N = atoi(argv[1]);
//MPI stuff
MPI_Init(&argc,&argv);
MPI_Comm_size(MPI_COMM_WORLD, &actorCount);
MPI_Comm_rank(MPI_COMM_WORLD, &echelon);

//Root process
if( echelon == ROOT )
{
	//Allocate memory for matrices
	matA = new int * [N];
	matB = new int * [N];
	matC = new int * [N];
	for( int i = 0; i < N; i++ )
	{
		matA[i] = new int [N];
		matB[i] = new int [N];
		matC[i] = new int [N];
	}

	int a;
	
	//Read in data from the file
	for( int i = 0; i < N; i++ )
	{
		for( int j = 0; j < N; j++ )
		{
			matA[i][j] = rand() % 1000;
			matB[i][j] = rand() % 1000;
		}
	}

	//start the clock
	start = MPI_Wtime();
	
	//multiply the matricies
	for( int i = 0; i < N; i++ )
	{
		for( int j = 0; j < N; j++ )
		{
			matC[i][j] = 0;
			for( int k = 0; k < N; k++ )
			{
				matC[i][j] += (matA[i][k] * matB[k][j]);
			}
		}
	}

	//end the clock
	end = MPI_Wtime();
	cout << end - start << endl;

	if( VERBOSE )
	{
		for( int i = 0; i < N; i++ )
		{
			for( int j = 0; j < N; j++ )
			{
				cout << matC[i][j] << '\t';
			}
			cout << endl;
		}
	}

}


MPI_Finalize();

return 0;

}
