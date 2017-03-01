#include <iostream>
#include "PIMFuncs.h"
using namespace std;

struct complex{
	float real;
	float imag;
};

int cal_pixel( complex c );

int main()
{
	int HEIGHT = 2000;
	int WIDTH = 2000;


	unsigned char ** myPix;
	myPix = new unsigned char * [WIDTH];
	for( int i = 0; i < WIDTH; i++ )
	{
		myPix[i] = new unsigned char [HEIGHT];
	}

	int real_max = 2;
	int real_min = -2;
	int imag_max = 2;
	int imag_min = -2;
	int disp_width = WIDTH;
	int disp_height = HEIGHT;

	complex p;
	p.real = 0.0;
	p.imag = 0.0;

	int color = 0;


	float scale_real = (float)(real_max - real_min)/disp_width;
	float scale_imag = (float)(imag_max - imag_min)/disp_height;

	for( int x = 0; x < disp_width; x++ )
	{
		for( int y = 0; y < disp_height; y++ )
		{
			p.real = real_min + ((float) x * scale_real);
			p.imag = imag_min + ((float) y * scale_imag);
			color = cal_pixel(p);
			myPix[x][y] = color;
		}
	}


	pim_write_black_and_white( "input",
			WIDTH,
			HEIGHT,
			(const unsigned char **) myPix);
			
}

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
