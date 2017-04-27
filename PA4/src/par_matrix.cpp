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
int ** contigAllocate(int rowSize, int colSize);

//Gets the left processor of given processor in the mesh
int getLeftProc( int rank, int meshSize );

//Gets the up processor of given processor in the mesh
int getUpProc( int rank, int meshSize );

//Gets the Row value in mesh
int getMeshRow( int rank, int meshSize );

//Gets the Col value in mesh
int getMeshCol( int rank, int meshSize );

//Print out the matrix
void printMatrix( int **table , int matSize);

//sets given matrix to all 0s
void initMatrix( int **table, int matSize);

//distributes the submatricies of the large matricies to the mesh
void partMatrix( int **bigMatA, int **bigMatB, int **smallMatA, int **smallMatB , int rank, int smallMatSize, int procCount, MPI_Status stat);

//shift left function
void shiftLeft(int **smallMatA, int **buffer, int leftNode, int matSize, MPI_Status stat, MPI_Request req );

//shift up function
void shiftUp(int **smallMatB, int **buffer, int upNode, int matSize, MPI_Status stat, MPI_Request req );

//multiples two matricies A and B and saves result into C
void seqMatMult( int **matrixA, int **matrixB, int **matrixC, int matSize );

//converts an array to a matrix
void convertArrToMat( int *arr, int **matrix, int matSize, int size);

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

//File data
ifstream fileA;
ifstream fileB;
fileA.open(argv[1]);
fileB.open(argv[2]);

//Matrix data
int **largeMatA;
int **largeMatB;
int **largeMatC;
int largeN;
int N;
int **matA;
int **matB;
int **matC;

int *resultArr;
int *subResult;

fileA >> largeN;
fileB >> largeN;

resultArr = new int [largeN*largeN];
//MPI stuff
MPI_Init(&argc,&argv);
MPI_Comm_size(MPI_COMM_WORLD, &actorCount);
MPI_Comm_rank(MPI_COMM_WORLD, &echelon);

//Calculates matrix sie for each node
N = largeN / sqrt(actorCount);

subResult = new int [N*N];

//Allocate memory for matrices
matA = contigAllocate(N,N);
matB = contigAllocate(N,N);
matC = contigAllocate(N,N);
largeMatA = contigAllocate(largeN,largeN);
largeMatB = contigAllocate(largeN,largeN);
largeMatC = contigAllocate(largeN,largeN);

int **tempMat = contigAllocate(N,N);

initMatrix(matC, N);

if( echelon == ROOT )
{
	for( int i = 0; i < largeN; i++ )
	{
		for( int j = 0; j < largeN; j++ )
		{
			fileA >> largeMatA[i][j];
			fileB >> largeMatB[i][j];
		}
	}
}

//distribute the file matrix to the mesh
partMatrix( largeMatA, largeMatB, matA, matB , echelon, N, actorCount, Stat);


//Mesh values and data
int rootP = sqrt(actorCount);

int Col = getMeshCol( echelon, rootP );
int Row = getMeshRow( echelon, rootP );

int leftProc = getLeftProc( echelon, rootP );
int upProc = getUpProc( echelon, rootP );

MPI_Barrier(MPI_COMM_WORLD);

//initial swaps

//Start counting
if( echelon == ROOT )
	start = MPI_Wtime();

//left shifts first
for( int i = 0; i < Row; i++) 
{
	if( Row > 0 )
		shiftLeft( matA, tempMat, leftProc, N, Stat, Request );
}

MPI_Barrier(MPI_COMM_WORLD);
//up shifts now
for( int i = 0; i < Col; i++) 
{
	if( Col > 0 )
		shiftUp( matB, tempMat, upProc, N, Stat, Request );
}


MPI_Barrier(MPI_COMM_WORLD);

//Now shift submatricies left and up
for( int multCnt = 0; multCnt < rootP; multCnt++ )
{

	seqMatMult(matA, matB, matC, N);

	MPI_Barrier(MPI_COMM_WORLD);

	//Swap left
	shiftLeft( matA, tempMat, leftProc, N, Stat, Request );

	MPI_Barrier(MPI_COMM_WORLD);


	//Swap up
	shiftUp( matB, tempMat, upProc, N, Stat, Request );

	MPI_Barrier(MPI_COMM_WORLD);
}

MPI_Barrier(MPI_COMM_WORLD);

if(echelon == ROOT )
	end = MPI_Wtime();

int k = 0;
for( int i = 0; i < N; i++ )
{
	for( int j = 0; j < N; j++ )
	{
		subResult[k] = matC[i][j];
		k++;
	}
}

MPI_Barrier(MPI_COMM_WORLD);

MPI_Gather( subResult, N*N, MPI_INT, resultArr, N*N, MPI_INT, ROOT, MPI_COMM_WORLD );


if( echelon == ROOT )
{
	convertArrToMat( resultArr, largeMatC, largeN, N);

	cout << "Matrix calculated in " << (end - start) << " seconds " << endl;

	cout << "Contents of matrix A: \n";
	printMatrix( largeMatA, largeN );
	cout << "Contents of matrix B: \n";
	printMatrix( largeMatB, largeN );
	cout << "Contents of matrix C: \n";
	printMatrix( largeMatC, largeN );
}

MPI_Finalize();

return 0;

}

//Contiguous allocation of 2D array
int** contigAllocate(int rowSize, int colSize)
{
	int** ptr = new int*[rowSize];
	int* pool = new int[rowSize*colSize];
	for( int i = 0; i < rowSize; ++i, pool += colSize )
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


void printMatrix( int **table , int matSize )
{
	for( int i = 0; i < matSize; i++ )
	{
		for( int j = 0; j < matSize; j++ )
		{
			cout << table[i][j] << ' ';
		}
		cout << endl;
	}
}


void initMatrix( int **table, int matSize)
{
	for( int i = 0; i < matSize; i++ )
	{
		for( int j = 0; j < matSize; j++ )
		{
			table[i][j] = 0;
		}
	}
}

void partMatrix( int **bigMatA, int **bigMatB, int **smallMatA, int **smallMatB , int rank, int smallMatSize, int procCount, MPI_Status stat)
{
	if( rank == ROOT )
	{
		int meshRowCounter = 0;
		int meshColCounter = 0;
		int rowOffset = 0;
		int colOffset = 0;

		meshColCounter++;
		colOffset += smallMatSize;

		for( int i = 1; i < procCount; i++ )
		{
			for( int j = 0; j < smallMatSize; j++ )
			{
				for( int k = 0; k < smallMatSize; k++ )
				{
					smallMatA[j][k] = bigMatA[j + rowOffset][k + colOffset];
					smallMatB[j][k] = bigMatB[j + rowOffset][k + colOffset];
				}
			}

			meshColCounter++;
			colOffset += smallMatSize;
			if( meshColCounter == smallMatSize + 1)
			{
				meshRowCounter++;
				rowOffset += smallMatSize;
				colOffset = 0;
				meshColCounter = 0;
			}

			MPI_Send(&(smallMatA[0][0]) , smallMatSize*smallMatSize, MPI_INT, i, 0, MPI_COMM_WORLD);
			MPI_Send(&(smallMatB[0][0]) , smallMatSize*smallMatSize, MPI_INT, i, 0, MPI_COMM_WORLD);
		}

		rowOffset = 0;
		colOffset = 0;
		//matrices for root
		for( int j = 0; j < smallMatSize; j++ )
		{
			for( int k = 0; k < smallMatSize; k++ )
			{

				smallMatA[j][k] = bigMatA[j + rowOffset][k + colOffset];
				smallMatB[j][k] = bigMatB[j + rowOffset][k + colOffset];
			}
		}

	}

	//Recv the matricies
	if( rank > ROOT )
	{
		MPI_Recv(&(smallMatA[0][0]) , smallMatSize*smallMatSize, MPI_INT, ROOT, 0, MPI_COMM_WORLD, &stat);
		MPI_Recv(&(smallMatB[0][0]) , smallMatSize*smallMatSize, MPI_INT, ROOT, 0, MPI_COMM_WORLD, &stat);
	}
}

void shiftLeft(int **smallMatA, int **buffer, int leftNode, int matSize, MPI_Status stat, MPI_Request req )
{
	MPI_Isend(&(smallMatA[0][0]), matSize*matSize, MPI_INT, leftNode, 0, MPI_COMM_WORLD, &req);	
	//recv
	MPI_Recv(&(buffer[0][0]), matSize*matSize, MPI_INT, MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, &stat);
	//wait
	MPI_Wait(&req, &stat);
	
	for( int i = 0; i < matSize; i++ )
	{
		for( int j = 0; j < matSize; j++ )
		{
			smallMatA[i][j] = buffer[i][j];
		}
	}
}

void shiftUp(int **smallMatB, int **buffer, int upNode, int matSize, MPI_Status stat, MPI_Request req )
{
	MPI_Isend(&(smallMatB[0][0]), matSize*matSize, MPI_INT, upNode, 0, MPI_COMM_WORLD, &req);	
	//recv
	MPI_Recv(&(buffer[0][0]), matSize*matSize, MPI_INT, MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, &stat);
	//wait
	MPI_Wait(&req, &stat);
	
	for( int i = 0; i < matSize; i++ )
	{
		for( int j = 0; j < matSize; j++ )
		{
			smallMatB[i][j] = buffer[i][j];
		}
	}
}

void seqMatMult( int **matrixA, int **matrixB, int **matrixC, int matSize )
{
	for( int i = 0; i < matSize; i++ )
	{
		for( int j = 0; j < matSize; j++ )
		{
			for( int k = 0; k < matSize; k++ )
			{
				matrixC[i][j] += (matrixA[i][k] * matrixB[k][j]);
			}
		}
	}

}


void convertArrToMat( int *arr, int **matrix, int matSize, int size)
{
	int rowOffset = 0;
	int colOffset = 0;
	int colCounter = 0;
	int index = 0;
	int rowIndex = 0;
	int colIndex = 0;

	for( int i = 0; i < matSize; i++ )
	{
		for( int j = 0; j < matSize; j++ )
		{
			matrix[rowIndex + rowOffset][colIndex + colOffset] = arr[index];
			if( colIndex == (size - 1 ) && rowIndex != (size - 1) )
			{
				rowIndex++;
				colIndex = 0;
			}
			else if( rowIndex == (size - 1 ) && colIndex == (size - 1) )
			{
				rowIndex = 0;
				colIndex = 0;
				if( colCounter == size )
				{
					colOffset = 0;
					rowOffset += size;
					colCounter = 0;
				}
				else
				{
				colCounter++;
				colOffset += size;
				}
			}
			else
			{
				colIndex++;
			}
			index++;
		}
	}
}
