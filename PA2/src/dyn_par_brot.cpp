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
int disp_height = 500; 
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

//Lord process
if( echelon == lord )
{
	//The strip to send to the vassals
	unsigned char strip[ disp_width ];

	//An archive that keeps track of the row each vassal is working on
	int archive[ actorCount ];

	//The row that was returned by the vassal
	int returnedRow;

	count = 0;
	row = 0;

	//Start sending out rows to vassals
	for( int actor = 1; actor < actorCount; actor++ )
	{
		MPI_Send( &row , 1 , MPI_INT , actor , data_tag , MPI_COMM_WORLD );

		//I record which row the vassal works on
		archive[ actor ] = row;
		count++;
		row++;
	}


	start = 0;
	start = MPI_Wtime();

	//Vassals come back to lord, must be told to continue or to terminate
	do {
		MPI_Recv( &strip , disp_width , MPI_UNSIGNED_CHAR , MPI_ANY_SOURCE , result_tag , MPI_COMM_WORLD , &Stat );
		count--;

		vassal = Stat.MPI_SOURCE;
		returnedRow = archive[ vassal ];

		//Copy the strip the lord received from the vassal into the image
		for( int dot = 0; dot < disp_width; dot++ )
		{
			fresco[ returnedRow ][ dot ] = strip[ dot ];
		}

		//If row < disp_height the there must be rows still left to compute
		if( row < disp_height )
		{
			//Tell the vassal that just returned to continue to work
			MPI_Send( &row , 1 , MPI_INT , vassal , data_tag , MPI_COMM_WORLD );
			archive[ vassal ] = row;
			row++;
			count++;
		}

		//Otherwise, there are no more rows left to compute
		else
		{
			//Tell the vassal that just returned to stop working
			MPI_Send( &count , 1 , MPI_INT , vassal , terminator_tag , MPI_COMM_WORLD );
		}


	//While there are still vassals working
	} while( count > 0 );

	end = MPI_Wtime();
	duration = end - start;
	cout << duration << endl;

	//Save the image
	pim_write_black_and_white( "dynamic_img",
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

	//The number passed into the cal_pixel function
	complex c;
	c.real = 0;
	c.imag = 0;

	//The lord tells me which row to compute
	MPI_Recv( &ordinate , 1 , MPI_INT , lord , MPI_ANY_TAG , MPI_COMM_WORLD , &Stat );

	//As long as the tag is appropriate, I shall continue to work
	while( Stat.MPI_TAG == data_tag )
	{
		c.imag = imag_min + ( (float) ordinate * scale_imag);

		//I calculate the mandelbrot data pixels for the row I am assigned
		for( abscissa = 0; abscissa < disp_width; abscissa++ )
		{
			c.real = real_min + ( (float) abscissa * scale_real);
			arr[ abscissa ] = cal_pixel( c );
		}

		//I send my hard work back to the lord
		MPI_Send( &arr , disp_width , MPI_UNSIGNED_CHAR , lord , result_tag , MPI_COMM_WORLD );

		//lord will tell me if there is still work to do or if the day is done
		MPI_Recv( &ordinate , 1 , MPI_INT , lord , MPI_ANY_TAG , MPI_COMM_WORLD , &Stat );
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
