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

//Image and calculation data
int real_max = 2;
int real_min = -2;
int imag_max = 2;
int imag_min = -2;
int disp_width = 1000;
int disp_height =1000;
float scale_real = (float)(real_max - real_min)/disp_width;
float scale_imag = (float)(imag_max - real_min)/disp_height;

//Time data
double start;
double end;
double duration;

//Load balancing data
//The lord commands the vassals
int lord = 0;


//The fresco is the image we are creating from this process
//It will have a mandelbrot saved to it
unsigned char ** fresco;
fresco = new unsigned char * [ disp_height ];
for( int i = 0; i < disp_height; i++ )
{
	fresco[ i ] = new unsigned char [ disp_width ];
}

int abscissa;
int ordinate;

complex c;


MPI_Init(&argc,&argv);
MPI_Comm_size(MPI_COMM_WORLD, &actorCount);
MPI_Comm_rank(MPI_COMM_WORLD, &echelon);

//Lord process
if( echelon == lord )
{
	duration = 0;
	c.real = 0.0;
	c.imag = 0.0;

	//Calculates the mandelbrot pixel by pixel using cal_pixel
	start = MPI_Wtime();
	for( abscissa = 0; abscissa < disp_width; abscissa++ )
	{
		for( ordinate = 0; ordinate < disp_height; ordinate++ )
		{
			c.real = real_min + ((float) abscissa * scale_real );
			c.imag = imag_min + ((float) ordinate * scale_imag );
			fresco[ordinate][abscissa] = cal_pixel( c );
		}
	}
	end = MPI_Wtime();
	duration = end - start;
	cout << "Image processing took: \n";
	cout << duration << endl;

	//Save the image
	pim_write_black_and_white( "seq_img",
				disp_width,
				disp_height,
				(const unsigned char **) fresco);



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
