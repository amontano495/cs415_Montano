#include <iostream>
#include <fstream>
#include "mpi.h"
#include <cstdio>
#include <cstring>
#include <stdlib.h>
#include <vector>
#define MAX_SIZE 1000
#define ROOT 0
using namespace std;


//Global to print out 
int VERBOSE = false;

//Fills array with random numbers
void RNG( int arr[] , int number_of_points );

//A simple bubble sort for a vector
void bubbleSort( vector<int> &arr );

int main (int argc, char *argv[])
{

int n_items = atoi( argv[ 1 ] );
int n_buckets = atoi( argv[ 2 ] );

if( argc > 3 ) 
{
	if( atoi( argv[ 3 ] ) == 1 )
	{
		VERBOSE = true;
	}
}

//Mpi variables
int actorCount;
int echelon;

//Time data
double start;
double end;

//A 2D array where the 2nd dim is each bucket
vector<int> buckets[ n_buckets ];

//An array to hold the numbers to be sorted
int *array;

//Expand the array to hold the numbers that will be sorted
array = new int [ n_items ];

RNG( array , n_items );

MPI_Init(&argc,&argv);
MPI_Comm_size(MPI_COMM_WORLD, &actorCount);
MPI_Comm_rank(MPI_COMM_WORLD, &echelon);

//Lord process
if( echelon == ROOT )
{
	start = MPI_Wtime();
	int bucket_index;

	//Scatter
	bool placed = false;
	int val;
	for( int i = 0; i < n_items; i++ )
	{
		//Places each value from the array into the appropriate bucket
		bucket_index = ( array[ i ] * n_buckets ) / MAX_SIZE;
		buckets[ bucket_index ].push_back( array [ i ] );
	}

	//This is just a simple bubble sort but called on each row of the 2D array
	for( int i = 0; i < n_buckets; i++ )
	{
		bubbleSort( buckets[ i ] );
	}

	end = MPI_Wtime();
	cout << end - start << endl;

}


MPI_Finalize();

return 0;

}

void RNG( int arr[] , int number_of_points )
{
	int i;
	long random_num;

	for( i = 0; i < number_of_points; i++ )
	{
		random_num = random();
		arr[ i ] = ( random_num % MAX_SIZE );
	}
}

void bubbleSort( vector<int> &arr )
{
	bool swapped = true;
	int j = 0;
	int tmp;
	int length = arr.size();
	while( swapped )
	{
		swapped = false;
		j++;
		for( int i = 0; i < (length - j); i++ )
		{
			if( arr[ i ] > arr[ i + 1 ] )
			{
				tmp = arr[ i ];
				arr[ i ] = arr[ i + 1 ];
				arr[ i + 1 ] = tmp;
				swapped = true;
			}
		}
	}
}
