#include <iostream>
#include <fstream>
#include "mpi.h"
#include <cstdio>
#include <cstring>
#include <stdlib.h>
#include <unistd.h>
#include <vector>
#include <algorithm>
#define	MAX_SIZE 1000
#define ROOT 0
using namespace std;


//Used to see the sorted data
bool VERBOSE = false;

//Fills array with random numbers
void RNG( int arr[] , int number_of_points );

//simple bubble sort for vectors
void bubbleSort( vector<int> &arr );


int main (int argc, char *argv[])
{

if( argc > 2 )
{
	if( atoi( argv[ 2 ] ) == 1 )
	{
		VERBOSE = true;
	}
}

int INPUT_SIZE;

INPUT_SIZE = atoi( argv[ 1 ] );

//Mpi variables
int actorCount, echelon;

//Time data
double start;
double end;

//Bucket variables and buffers
int n_items, numbersPerProc, bucket_size, bigBucketSize, maxBucketSize, n_buckets;

//Buffers to send data between processors
int *fileData, *localbuffer, *sortedBuffer, *sendBuffer;

//Vector to hold data as a big bucket
vector<int> bigBucket;

n_items = INPUT_SIZE;

//Expand the array to hold the numbers that will be sorted
fileData = new int [ n_items ];

//File the array with random data
RNG( fileData , INPUT_SIZE );

//Easy to be greater than this
maxBucketSize = -1;

//Initalize mpi
MPI_Init(&argc,&argv);
MPI_Comm_size(MPI_COMM_WORLD, &actorCount);
MPI_Comm_rank(MPI_COMM_WORLD, &echelon);

n_buckets = actorCount;
//Vector to hold data as a little bucket
vector<int> littleBuckets[ n_buckets ];

//array that will hold the largest size little bucket of each processor
int bucketSizes[ actorCount ];

//buffers to hold varying sizes of returning data
//used for MPI_Gatherv
int displs[ actorCount ];
int recvcnt[ actorCount ];

//buffers to hold varying sizes of sending data
//used for MPI_Scatterv
int send_displs[ actorCount ];
int sendcnt[ actorCount ];

numbersPerProc = n_items / actorCount;

//Get numbers from the file
if( echelon == ROOT )
{
	int sum = 0;
	//Setting up the displacements and send counts for root when scatterv occurs
	if( n_items % actorCount != 0 )
	{
		sendcnt[ 0 ] = ( n_items / actorCount ) + ( n_items % actorCount );
	}
	else
	{
		sendcnt[ 0 ] = numbersPerProc;
	}

	for( int i = 1; i < actorCount; i++ )
	{
		sendcnt[ i ] = numbersPerProc;
	}

	send_displs[ 0 ] = n_items - sendcnt[ 0 ];
	for( int i = 1; i < actorCount; i++ )
	{
		send_displs[ i ] = sum;
		sum += sendcnt[ i ];
	}

	numbersPerProc = sendcnt[ 0 ];
}


localbuffer = new int [ numbersPerProc ];

//scatter the data to each processor
MPI_Barrier( MPI_COMM_WORLD );
MPI_Scatterv( fileData, sendcnt, send_displs, MPI_INT, localbuffer, numbersPerProc, MPI_INT, 0, MPI_COMM_WORLD );

//Start timing now that each bucket has data
start = MPI_Wtime();

//locally scatter data into the little buckets
int bucket_index;
bool placed = false;
int val = 0;

for( int i = 0; i < numbersPerProc; i++ )
{
	bucket_index = ( localbuffer[ i ] * n_buckets ) / MAX_SIZE;
	littleBuckets[ bucket_index ].push_back( localbuffer[ i ] );
}

//Find the largest bucket size of from each processor
int bucketSize;
for( int i = 0; i < n_buckets; i++ )
{
	bucketSize = littleBuckets[ i ].size();
	if( bucketSize > maxBucketSize )
	{
		maxBucketSize = bucketSize;
	}
}


//Each processor sends its largest bucket size to root
MPI_Barrier( MPI_COMM_WORLD );
MPI_Gather( &maxBucketSize, 1, MPI_INT, bucketSizes, 1, MPI_INT, 0, MPI_COMM_WORLD );


int tempMax = -1;
//Root finds bucket size to give every processor for uniformity
if( echelon == ROOT )
{
	for( int i = 0; i < actorCount; i++ )
	{
		if( bucketSizes[ i ] > tempMax )
		{
			tempMax = bucketSizes[ i ];
		}
	}

	//fill the array with the max size for easy scatter
	for( int i = 0; i < actorCount; i++ )
	{
		bucketSizes[ i ] = tempMax;
	}
}


MPI_Barrier( MPI_COMM_WORLD );
MPI_Scatter( bucketSizes, 1, MPI_INT, &maxBucketSize, 1, MPI_INT, 0, MPI_COMM_WORLD );

int inBuckets[ n_buckets ][ maxBucketSize ];
int outBuckets[ n_buckets ][ maxBucketSize ];

//Resize the buckets to have uniform size
//Copy the vectors into the 2D array
//This makes alltoall easier
for( int i = 0; i < n_buckets; i++ )
{
	littleBuckets[ i ].resize( maxBucketSize );
	copy( littleBuckets[ i ].begin(), littleBuckets[ i ].end(), outBuckets[ i ] );
}

//Give i bucket to i processor but wait till all processors ready
MPI_Barrier( MPI_COMM_WORLD );
MPI_Alltoall( outBuckets, maxBucketSize, MPI_INT, inBuckets, maxBucketSize, MPI_INT, MPI_COMM_WORLD );


//Parse thru little buckets and empty into the big bucket
for( int i = 0; i < n_buckets; i++ )
{
	for( int j = 0; j < maxBucketSize; j++ )
	{
		if( inBuckets[ i ][ j ] > 0 )
		{
			bigBucket.push_back( inBuckets[ i ][ j ] );
		}
	}
}


//Sort the big bucket and convert to a normal int array
//bubbleSort( bigBucket );
sort( bigBucket.begin(), bigBucket.end() );
//Each bucket is now sorted, stop timing
if( echelon == ROOT )
{
	end = MPI_Wtime();
	cout << (end - start) << endl;
}

sendBuffer = new int [ bigBucket.size() ];
copy( bigBucket.begin() , bigBucket.end(), sendBuffer );

//send the sizes of the big buckets to the root
bigBucketSize = bigBucket.size();
MPI_Gather( &bigBucketSize, 1, MPI_INT, recvcnt, 1, MPI_INT, 0, MPI_COMM_WORLD );

//root calculates the displacements and prepares to receive the data
if( echelon == ROOT )
{
	displs[ 0 ] = 0;
	for( int i = 1; i < actorCount; i++ )
	{
		displs[ i ] = displs[ i - 1 ] + recvcnt[ i - 1 ];
	}
	sortedBuffer = new int [ n_items ];
}

//Send big buckets from each processor to root but wait till everyone is ready first
MPI_Barrier( MPI_COMM_WORLD );
MPI_Gatherv( sendBuffer, bigBucketSize, MPI_INT, sortedBuffer, (const int*) recvcnt, (const int*) displs, MPI_INT, 0, MPI_COMM_WORLD );

if( echelon == ROOT )
{
	if( VERBOSE )
	{
		for( int i = 0; i < n_items; i++ )
		{
			cout << sortedBuffer[ i ] << endl;
		}
	}
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
