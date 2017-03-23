#include <iostream>
#include "mpi.h"
#include <cstdio>
#include <cstring>
#include <stdlib.h>
using namespace std;

#define WRITE_FLAGS "w"

struct complex{
	float real;
	float imag;
};


bool pim_write_black_and_white(const char * const fileName,
                               const int width,
                               const int height,
                               const unsigned char * pixels);

bool pim_write_black_and_white(const char * const fileName,
                               const int width,
                               const int height,
                               const unsigned char ** pixels);

int cal_pixel( complex c );

int main (int argc, char *argv[])
{

//MPI information
int actorCount, echelon;
MPI_Status Stat;

//Image and calculation data
int real_max = 2;
int real_min = -2;
int imag_max = 2;
int imag_min = -2;
int disp_width = 500;
int disp_height =500;
float scale_real = (float)(real_max - real_min)/disp_width;
float scale_imag = (float)(imag_max - real_min)/disp_height;

//Time data
double start;
double end;
double duration;

//Load balancing data
int count;
int row;

//The lord commands the vassals
int vassal;
int lord = 0;

//Tags that keep track of when to do certain processes
int data_tag, source_tag, terminator_tag, result_tag;

data_tag = 0;
result_tag = 2;
terminator_tag = 3;

//flag to see if there are an odd numbered of processors
bool oddProcs;

//If there is an odd number of slaves, one will have to do the left over rows
int remainingRows;

//The fresco is the image we are creating from this process
//It will have a mandelbrot saved to it
unsigned char ** fresco;
fresco = new unsigned char * [ disp_height ];
for( int i = 0; i < disp_height; i++ )
{
	fresco[ i ] = new unsigned char [ disp_width ];
}


MPI_Init(&argc,&argv);
MPI_Comm_size(MPI_COMM_WORLD, &actorCount);
MPI_Comm_rank(MPI_COMM_WORLD, &echelon);

int slaves = actorCount - 1;

if( slaves % 2 == 0 )
{
	oddProcs = false;
}
else
{
	oddProcs = true;
}

int rowsPerProcess = disp_height / slaves;

if( oddProcs )
{
	remainingRows = disp_height - (rowsPerProcess * slaves );
}


//Lord process
if( echelon == lord )
{
	//The strip to send to the vassals
	unsigned char strip[ disp_width ];

	//The row that was returned by the vassal
	int returnedRow;

	count = 0;
	row = 0;

	//Start sending out rows to vassals
	for( int actor = 1; actor < actorCount; actor++ )
	{
		if( oddProcs && (actor == actorCount - 1) )
		{
			row += remainingRows;
		}

		MPI_Send( &row , 1 , MPI_INT , actor , data_tag , MPI_COMM_WORLD );
		row += rowsPerProcess;
	}

	start = 0;
	start = MPI_Wtime();

	//Copy the strip the lord received from the vassal into the image
	for( int i = 0; i < (rowsPerProcess * slaves); i++ )
	{
		MPI_Recv( &strip , disp_width , MPI_UNSIGNED_CHAR , MPI_ANY_SOURCE , MPI_ANY_TAG , MPI_COMM_WORLD , &Stat );

		returnedRow = Stat.MPI_TAG;

		for( int dot = 0; dot < disp_width; dot++ )
		{
			fresco[ returnedRow ][ dot ] = strip[ dot ];
		}
	}

	end = MPI_Wtime();
	duration = end - start;
	cout << duration << endl;

	//Save the image
	pim_write_black_and_white( "static_img",
				disp_width,
				disp_height,
				(const unsigned char **) fresco);
}

//Vassal processes
if( echelon > lord )
{
	//The array contains the pixel data to return to the lord
	unsigned char arr[ disp_width ];

	//The coordinates that help calculate the mandelbrot
	int abscissa;
	int ordinate;
	int row;

	//The number passed into the cal_pixel function
	complex c;
	c.real = 0;
	c.imag = 0;

	//The lord tells me which row to compute
	MPI_Recv( &row , 1 , MPI_INT , lord , data_tag , MPI_COMM_WORLD , &Stat );

	//I calculate the mandelbrot data pixels for the row I am assigned
	for( ordinate = row; ordinate < (row + rowsPerProcess); ordinate++ )
	{
		c.imag = imag_min + ( (float) ordinate * scale_imag);

		for( abscissa = 0; abscissa < disp_width; abscissa++ )
		{
			c.real = real_min + ( (float) abscissa * scale_real);
			arr[ abscissa ] = cal_pixel( c );
		}

		//I send my hard work back to the lord
		MPI_Send( &arr , disp_width , MPI_UNSIGNED_CHAR , lord , ordinate , MPI_COMM_WORLD );
	}

}

MPI_Finalize();

return 0;

}

//Calculates the pixel data based on given complex number
//Given by the textbook
int cal_pixel( complex c )
{
	int count, max_iter;
	complex z;
	float temp, lengthsq;
	max_iter = 256;
	z.real = 0;
	z.imag = 0;
	count = 0;
	do {
		temp = z.real * z.real - z.imag * z.imag + c.real;
		z.imag = 2 * z.real * z.imag + c.imag;
		z.real = temp;
		lengthsq = z.real * z.real + z.imag * z.imag;
		count++;
	} while( (lengthsq < 4.0) && (count < max_iter));
		return count;
}


//Saves the unsigned char * data to the row in the imagee
//Given by instructor
bool pim_write_black_and_white(const char * const fileName,
                               const int width,
                               const int height,
                               const unsigned char * pixels)
{
  FILE * fp = fopen(fileName, WRITE_FLAGS);

  if (!fp) return false;
  fprintf(fp, "P5\n%i %i 255\n", width, height);
  fwrite(pixels, width * height, 1, fp);
  fclose(fp);

  return true;
}

//Saves the unsigned char ** data to the image
//Given by instructor
bool pim_write_black_and_white(const char * const fileName,
                               const int width,
                               const int height,
                               const unsigned char ** pixels)
{
  int i;
  bool ret;
  unsigned char * t = new unsigned char[width * height];

  for (i = 0; i < height; ++i) memcpy(t + width * i, pixels[i], width);
  ret = pim_write_black_and_white(fileName, width, height, t);
  delete [] t;
  return ret;
}
