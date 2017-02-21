/******************************************************************************
* FILE: PA01.c
* AUTHOR: Adam Montano
* LAST REVISED: 02/20/2016
******************************************************************************/
#include "mpi.h"
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int main (int argc, char *argv[])
{

int numtasks, rank, dest, source, tag=1;
int inmsg, outmsg=10;
MPI_Status Stat;
char hostname[MPI_MAX_PROCESSOR_NAME];
int len;
double start;
double end;
double duration = 0;
int i;
int attempts = 10;


MPI_Init(&argc,&argv);
MPI_Comm_size(MPI_COMM_WORLD, &numtasks);
MPI_Comm_rank(MPI_COMM_WORLD, &rank);
MPI_Get_processor_name(hostname, &len);

if(rank == 0)
{
	dest = 1;
	source = 1;

	for( i = 0; i < attempts; i++ )
	{
		start = MPI_Wtime();
		 
		MPI_Send(&outmsg, 1, MPI_INT, dest, tag, MPI_COMM_WORLD);
		MPI_Recv(&inmsg, 1, MPI_INT, source, tag, MPI_COMM_WORLD, &Stat);

		end = MPI_Wtime();
		duration += (end - start);
	}
	duration = duration / attempts;
	printf("time between %d and %d is: %f\n", 0 , 1 , duration);
}


else if(rank == 1)
{
	dest = 0;
	source = 0;

	for( i = 0; i < attempts; i++ )
	{
		MPI_Recv(&inmsg, 1, MPI_INT, source, tag, MPI_COMM_WORLD, &Stat);
		MPI_Send(&outmsg, 1, MPI_INT, dest, tag, MPI_COMM_WORLD);
	}
}


MPI_Finalize();

return 0;
}
