#include <iostream>
#include <fstream>
#include "mpi.h"
#include <cstdio>
#include <cstring>
#include <stdlib.h>
using namespace std;

#define WRITE_FLAGS "w"

int main (int argc, char *argv[])
{

//Mpi variables
int actorCount;
int echelon;

//Time data
double start;
double end;
double duration;

//The lord is the only required node
int lord = 0;


//The amount of numbers to sort
int n_items;

//An iterator through the file
int i;

//The amount of buckets
int n_buckets = 2;

//The max interval of the numbers to sort
int MAX_SIZE = 1000;

//The size of the bucket
int bucket_size;

//A 2D array where the 2nd dim is each bucket
int ** buckets;

//An array to hold the numbers to be sorted
int *array;

//Reading in the file...
ifstream file;
file.open( argv[ 1 ] );
file >> n_items;

//Expand the array to hold the numbers that will be sorted
array = new int [ n_items ];

bucket_size = MAX_SIZE / n_buckets;

//Initializing the 2D array, -1 means null
buckets = new int * [ n_buckets ];
for( int i = 0; i <  n_buckets; i++ )
{
	buckets[ i ] = new int [ bucket_size ];
	for( int j = 0; j < bucket_size; j++ )
	{
		buckets[ i ][ j ] = -1;
	}
}


//Filling the array with the numbers from file
i = 0;
while( i < n_items )
{
	file >> array[ i ];
	i++;
}


MPI_Init(&argc,&argv);
MPI_Comm_size(MPI_COMM_WORLD, &actorCount);
MPI_Comm_rank(MPI_COMM_WORLD, &echelon);

//Lord process
if( echelon == lord )
{
	start = MPI_Wtime();
	int bucket_index;
	int k = 0;

	//Scatter
	bool placed = false;
	int val;
	for( int i = 0; i < n_items; i++ )
	{
		//Places each value from the array into the appropriate bucket
		bucket_index = ( array[ i ] * n_buckets ) / MAX_SIZE;
		val = 0;
		while( placed == false and val < bucket_size )
		{
			//This means the current stored value is null and ok to replace
			if( buckets[ bucket_index ][ val ] == -1 )
			{
				buckets[ bucket_index ][ val ] = array[ i ];
				placed = true;
			}
			val++;
		}
		placed = false;
	}

	//Sort
	int temp;
	int x = 0;
	//This is just a simple bubble sort but called on each row of the 2D array
	//In this context the rows are the buckets
	for( int i = 0; i < n_buckets; i++ )
	{
		x = 0;
		while( x < bucket_size )
		{
			if( buckets[ i ][ x ] > buckets[ i ][ x + 1 ] 
				and buckets[ i ][ x + 1 ] > -1 
				and buckets[ i ][ x ] > -1 )
			{
				temp = buckets[ i ][ x ];
				buckets[ i ][ x ] = buckets[ i ][ x + 1 ];
				buckets[ i ][ x + 1 ] = temp;
				x = 0;
			}

			else
			{
				x++;
			}
		}
	}

	//Gather
	//This merges the data from the buckets back into the array now that everything is sorted
	k = 0;
	for( int i = 0; i < n_buckets; i++ )
	{
		for( int j = 0; j < bucket_size; j++ )
		{
			if( buckets[ i ][ j ] > 0 and k < n_items)
			{
				array[ k ] = buckets[ i ][ j ];
				k++;
			}
		}
	}

	end = MPI_Wtime();
	cout << end - start << endl;

}


MPI_Finalize();

return 0;

}
