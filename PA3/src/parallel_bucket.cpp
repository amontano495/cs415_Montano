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
#define n_buckets 4
#define ROOT 0
using namespace std;

int main (int argc, char *argv[])
{

//Mpi variables
int actorCount, echelon;

//Time data
double start;
double end;
double duration;

//Bucket variables and buffers
int n_items, numbersPerProc, bucket_size, bigBucketSize;

//Buffers to send data between processors
int *fileData, *localbuffer, *sortedBuffer, *sendBuffer;

//Vector to hold data as a big bucket
vector<int> bigBucket;

//Reading in the file...
ifstream file;
file.open( argv[ 1 ] );
file >> n_items;

//Expand the array to hold the numbers that will be sorted
fileData = new int [ n_items ];

//biggest possible size a litle bucket could be
bucket_size = n_items;

int inBuckets[ n_buckets ][ bucket_size ];
int outBuckets[ n_buckets ][ bucket_size ];;

//the value could never be negative
for( int i = 0; i < n_buckets; i++ )
{
	for( int j = 0; j < bucket_size; j++ )
	{
		inBuckets[ i ][ j ] = -1;
		outBuckets[ i ][ j ] = -1;
	}
}

//Initalize mpi
MPI_Init(&argc,&argv);
MPI_Comm_size(MPI_COMM_WORLD, &actorCount);
MPI_Comm_rank(MPI_COMM_WORLD, &echelon);

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
	//Filling the array with the numbers from file
	for( int i = 0; i < n_items; i++ )
	{
		file >> fileData[ i ];
	}
	file.close();
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
/*
	for( int i = 1; i < actorCount; i++ )
	{
		send_displs[ i ] = send_displs[ i - 1] + sendcnt[ i - 1 ];
	}
*/
	numbersPerProc = sendcnt[ 0 ];
}


localbuffer = new int [ numbersPerProc ];

MPI_Barrier( MPI_COMM_WORLD );
//scatter the data to each processor
MPI_Scatterv( fileData, sendcnt, send_displs, MPI_INT, localbuffer, numbersPerProc, MPI_INT, 0, MPI_COMM_WORLD );

sleep( echelon );
cout << "local buffer for processor " << echelon << " :";
for( int i = 0; i < numbersPerProc; i++ )
{
	cout << localbuffer[ i ] << '\t';
}
cout << endl;

//locally scatter data into the little buckets
int bucket_index;
bool placed = false;
int val = 0;

for( int i = 0; i < numbersPerProc; i++ )
{
	bucket_index = ( localbuffer[ i ] * n_buckets ) / MAX_SIZE;
	while( !placed and val < bucket_size )
	{
		if( outBuckets[ bucket_index ][ val ] == -1 )
		{
			outBuckets[ bucket_index ][ val ] = localbuffer[ i ];
			placed = true;
		}
		val++;
	}
	placed = false;
	val = 0;
}


//Give i bucket to i processor but wait till all processors ready
MPI_Barrier( MPI_COMM_WORLD );
MPI_Alltoall( outBuckets, bucket_size, MPI_INT, inBuckets, bucket_size, MPI_INT, MPI_COMM_WORLD );


//Parse thru little buckets and empty into the big bucket
for( int i = 0; i < n_buckets; i++ )
{
	for( int j = 0; j < bucket_size; j++ )
	{
		if( inBuckets[ i ][ j ] != -1 )
		{
			bigBucket.push_back( inBuckets[ i ][ j ] );
		}
	}
}


//Sort the big bucket and convert to a normal int array
sort( bigBucket.begin(), bigBucket.end() );
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
	for( int i = 0; i < n_items; i++ )
	{
		cout << sortedBuffer[ i ] << endl;
	}
}


MPI_Finalize();

return 0;

}
