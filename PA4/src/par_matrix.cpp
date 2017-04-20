#include <iostream>
#include <fstream>
#include "mpi.h"
#include <cstdio>
#include <cstring>
#include <stdlib.h>
#include <vector>
#include <math.h>
#include <unistd.h>
#define ROOT 0
using namespace std;

//allocates a 2D array with contigious memory
//important for MPI sends and recvs
template <typename T>
T** create2DArray(unsigned nrows, unsigned ncols);

//Gets the left processor of given processor in the mesh
int getLeftProc( int rank, int meshSize );

//Gets the up processor of given processor in the mesh
int getUpProc( int rank, int meshSize );

//Gets the Row value in mesh
int getMeshRow( int rank, int meshSize );

//Gets the Col value in mesh
int getMeshCol( int rank, int meshSize );

//Global to print out 
int VERBOSE = false;

int main (int argc, char *argv[])
{

//Mpi variables
int actorCount;
int echelon;
MPI_Status Stat;
MPI_Request Request;

//Time data
double start;
double end;

//Matrix data
int largeN;
int N;
int **matA;
int **matB;
int **matC;

//Size given by command line
largeN = atoi(argv[1]);

//MPI stuff
MPI_Init(&argc,&argv);
MPI_Comm_size(MPI_COMM_WORLD, &actorCount);
MPI_Comm_rank(MPI_COMM_WORLD, &echelon);

//Calculates matrix sie for each node
N = largeN / sqrt(actorCount);

//Allocate memory for matrices
matA = create2DArray<int>(N,N);
matB = create2DArray<int>(N,N);
matC = create2DArray<int>(N,N);

int **tempMat = create2DArray<int>(N,N);

//Set matC to all 0s
for( int i = 0; i < N; i++ )
{
	for( int j = 0; j < N; j++ )
	{
		matC[i][j] = 0;
	}
}

srand(time(0));
//Root process
//Generate random values for matricies
//sends the matricies to nodes
if( echelon == ROOT )
{
	int a;
	int b;
	for( int i = 1; i < actorCount; i++ )
	{
		for( int j = 0; j < N; j++ )
		{
			for( int k = 0; k < N; k++ )
			{
				a = rand() % 10;
				b = rand() % 10;
				matA[j][k] = a;
				matB[j][k] = b;
			}
		}
		MPI_Send(&(matA[0][0]) , N*N, MPI_INT, i, 0, MPI_COMM_WORLD);
		MPI_Send(&(matB[0][0]) , N*N, MPI_INT, i, 0, MPI_COMM_WORLD);
	}

	//matrices for root
	for( int j = 0; j < N; j++ )
	{
		for( int k = 0; k < N; k++ )
		{
			a = rand() % 10;
			b = rand() % 10;
			matA[j][k] = a;
			matB[j][k] = b;
		}
	}
}

//Recv the matricies
if( echelon > ROOT )
{
	MPI_Recv(&(matA[0][0]) , N*N, MPI_INT, ROOT, 0, MPI_COMM_WORLD, &Stat);
	MPI_Recv(&(matB[0][0]) , N*N, MPI_INT, ROOT, 0, MPI_COMM_WORLD, &Stat);
}



MPI_Barrier(MPI_COMM_WORLD);


//Mesh values and data
int rootP = sqrt(actorCount);

int Col = getMeshCol( echelon, rootP );
int Row = getMeshRow( echelon, rootP );

int leftProc = getLeftProc( echelon, rootP );
int upProc = getUpProc( echelon, rootP );

MPI_Barrier(MPI_COMM_WORLD);

//initial swaps

//Start counting
start = MPI_Wtime();
MPI_Barrier(MPI_COMM_WORLD);

//left shifts first
for( int i = 0; i < Row; i++ )
{
	MPI_Isend(&(matA[0][0]), N*N, MPI_INT, leftProc, 0, MPI_COMM_WORLD, &Request);	
	//recv
	MPI_Recv(&(tempMat[0][0]), N*N, MPI_INT, MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, &Stat);
	//wait
	MPI_Wait(&Request, &Stat);

	//copy data
	if( Row > 0 )
	{
		for( int i = 0; i < N; i++ )
		{
			for( int j = 0; j < N; j++ )
			{
				matA[i][j] = tempMat[i][j];
			}
		}
	}
} 

MPI_Barrier(MPI_COMM_WORLD);

//up shifts
for( int i = 0; i < Col; i++ )
{
	//send
	MPI_Isend(&(matB[0][0]), N*N, MPI_INT, upProc, 0, MPI_COMM_WORLD, &Request);
	//recv
	MPI_Recv(&(tempMat[0][0]), N*N, MPI_INT, MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, &Stat);
	//wait
	MPI_Wait(&Request, &Stat);

	//copy dat
	if( Col > 0 )
	{
		for( int i = 0; i < N; i++ )
		{
			for( int j = 0; j < N; j++ )
			{
				matB[i][j] = tempMat[i][j];
			}
		}
	}
}


MPI_Barrier(MPI_COMM_WORLD);

//Now shift submatricies left and up
for( int multCnt = 0; multCnt < rootP; multCnt++ )
{
	//Multiply and save into C
	for( int i = 0; i < N; i++ )
	{
		for( int j = 0; j < N; j++ )
		{
			for( int k = 0; k < N; k++ )
			{
				matC[i][j] += (matA[i][k] * matB[k][j]);
			}
		}
	}

	MPI_Barrier(MPI_COMM_WORLD);

	//Swap left
	MPI_Isend(&(matA[0][0]), N*N, MPI_INT, leftProc, 0, MPI_COMM_WORLD, &Request);	
	MPI_Recv(&(tempMat[0][0]), N*N, MPI_INT, MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, &Stat);
	MPI_Wait(&Request, &Stat);
	
	for( int i = 0; i < N; i++ )
	{
		for( int j = 0; j < N; j++ )
		{
			matA[i][j] = tempMat[i][j];
		}
	}

	MPI_Barrier(MPI_COMM_WORLD);


	//Swap up
	MPI_Isend(&(matB[0][0]), N*N, MPI_INT, upProc, 0, MPI_COMM_WORLD, &Request);	
	MPI_Recv(&(tempMat[0][0]), N*N, MPI_INT, MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, &Stat);
	MPI_Wait(&Request, &Stat);

	for( int i = 0; i < N; i++ )
	{
		for( int j = 0; j < N; j++ )
		{
			matB[i][j] = tempMat[i][j];
		}
	}
	MPI_Barrier(MPI_COMM_WORLD);
}

MPI_Barrier(MPI_COMM_WORLD);
end = MPI_Wtime();

if(echelon == 0)
{
	cout << (end - start) << endl;
}

MPI_Finalize();

return 0;

}

//Contiguous allocation of 2D array
template <typename T>
T** create2DArray(unsigned nrows, unsigned ncols)
{
	T** ptr = new T*[nrows];
	T* pool = new T[nrows*ncols];
	for( unsigned i = 0; i < nrows; ++i, pool += ncols )
		ptr[i] = pool;
	return ptr;
}


int getLeftProc( int rank, int meshSize )
{
	int left;
	if( rank % meshSize == 0 )
	{
		left = rank + (meshSize - 1 );
	}
	else
	{
		left = rank - 1;
	}

	return left;
}

int getUpProc( int rank, int meshSize )
{
	int up;
	if( rank < meshSize )
	{
		up = rank + meshSize*(meshSize - 1 );
	}
	else
	{
		up = rank - meshSize;
	}

	return up;
}

int getMeshRow( int rank, int meshSize )
{
	int row = rank / meshSize;
	return row;
}

int getMeshCol( int rank, int meshSize )
{
	int col = rank % meshSize;
	return col;
}
