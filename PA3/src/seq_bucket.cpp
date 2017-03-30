#include <iostream>
#include <fstream>
#include "mpi.h"
#include <cstdio>
#include <cstring>
#include <stdlib.h>
using namespace std;

#define WRITE_FLAGS "w"
void bubbleSort( int array[] , int size );


int main (int argc, char *argv[])
{

int actorCount;
int echelon;

//Time data
double start;
double end;
double duration;

//Load balancing data
//The lord commands the vassals
int lord = 0;


int n_items;
int i;

int n_buckets = 16;
int MAX_SIZE = 1000;
int bucket_size;

int ** buckets;
int *array;

ifstream file;
file.open( argv[ 1 ] );
file >> n_items;

array = new int [ n_items ];

bucket_size = MAX_SIZE / n_buckets;

buckets = new int * [ n_buckets ];
for( int i = 0; i <  n_buckets; i++ )
{
	buckets[ i ] = new int [ bucket_size ];
	for( int j = 0; j < bucket_size; j++ )
	{
		buckets[ i ][ j ] = -1;
	}
}

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
		bucket_index = ( array[ i ] * n_buckets ) / MAX_SIZE;
		val = 0;
		while( placed == false and val < bucket_size )
		{
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

	for( int i = 0; i < n_items; i++ )
	{
		//cout << array[ i ] << endl;
	}
	end = MPI_Wtime();
	cout << end - start << endl;

}


MPI_Finalize();

return 0;

}

void bubbleSort( int array[] , int size ) 
{
	int i, temp;
	for( int i = 0; i < size; i++ )
	{
		if( array[ i ] > array[ i + 1 ] && array[ i ] != -1 )
		{
			temp = array[ i ];
			array[ i ] = array[ i + 1 ];
			array[ i + 1 ] = temp;
		}
	}

	for( int i = 0; i < size; i++ )
	{
		if( array[ i ] > -1 )
		{
			cout << array[ i ] << " ";
		}
	}
	cout << endl;
}
