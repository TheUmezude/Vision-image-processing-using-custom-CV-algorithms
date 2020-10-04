//The functions in techlead.cpp and techlead.h, were all created and implemented by the members of the group

#include "image_transfer3.h"
#include "techlead.h"
#include <Windows.h>
#include <stdio.h>
#include <stdlib.h>
#include "timer.h"
#include <math.h>
//serial libraries
#include <cstdio>
#include "serial_com.h"
#include <iostream>
#include <strstream> // for string streams
#include <conio.h>

using namespace std;

int draw_point_RGB(image &rgb, int ip, int jp, int R, int G, int B)
{
	ibyte *p;
	int i, j, w = 2, pixel;

	// initialize pointer
	p = rgb.pdata;

	if (rgb.type != RGB_IMAGE) {
		printf("\nerror in draw_point_RGB: input type not valid!");
		return 1;
	}

	// limit out of range values
	if (ip < w) ip = w;
	if (ip > rgb.width - w - 1) ip = rgb.width - w - 1;
	if (jp < w) jp = w;
	if (jp > rgb.height - w - 1) jp = rgb.height - w - 1;

	for (i = -w; i <= w; i++) {
		for (j = -w; j <= w; j++) {
			pixel = rgb.width*(jp + j) + (ip + i);
			p[3 * pixel] = B;
			p[3 * pixel + 1] = G;
			p[3 * pixel + 2] = R;
		}
	}

	return 0;
}

double object_radius(double ic, double jc, double beta, int label_number, image &label_image, image &rgb)
{

	//All it does is calculate the radius( and return it)
	//It doesnt get called in the main
	//It gets called in object_theta(...)

	double r = 0.0, dr, r_max;
	int i, j, k, width, height;
	i2byte *pl;

	width = label_image.width;
	height = label_image.height;

	pl = (i2byte *)label_image.pdata;

	dr = 0.5; // use 0.5 pixels just to be sure
	r_max = 70; // limit the max object radius size to something reasonable

	for (r = dr; r < r_max; r += dr) {
		i = (int)(ic + r*cos(beta));
		j = (int)(jc + r*sin(beta));

		// limit i and j in case it gets out of bounds -> wild pointer
		if (i < 0) i = 0;
		if (i >= width) i = width;
		if (j < 0) j = 0;
		if (j >= height) j = height;

		// convert i,j to image coord k
		k = j*width + i;

		// check when label is done -> r is just beyond the object radius
		if (pl[k] != label_number) break;
	}

	// mark point in rgb image for testing / debugging
	draw_point_RGB(rgb, i, j, 0, 255, 0);

	return r;
}

double object_theta(double ic, double jc, int label_number, image &label_image, image &rgb)
{
	double th, dth, th_max = 0.0, r_max = 0.0, r;
	int n = 45, i, j;

	dth = 3.14159 / n;

	for (th = 0.0; th < 2 * 3.141591; th += dth) {
		// call your homies !
		r = object_radius(ic, jc, th, label_number, label_image, rgb);
		if (r > r_max) {
			r_max = r;
			th_max = th;
		}
	}

	// mark point in rbg for debugging / testing
	i = (int)(ic + r_max*cos(th_max));
	j = (int)(jc + r_max*sin(th_max));

	//Tell us coordinates of the point
	printf("\nCoordinates of largest radius from centroid is (%d,%d)", i, j);
	draw_point_RGB(rgb, i, j, 255, 0, 0);
	draw_point_RGB(rgb, (int)ic, (int)jc, 0, 255, 255);

	return th_max;
}


double shape_detect(double ic, double jc, int label_number, image &label_image, image &rgb)
{
	//Goal: this function calculates if the label is a circle, square, ir rectangle.

	//Inputs:
	//ic,jc, 
	//label_number
	//label image
	//Define variables
	int i;
	double b1, b2, b3; //beta angles
	double r1, r2, r3; //radii at beta angles
	double max, min; //max and min radii values
	double r_max; //max radius
	double theta_max; //theta max


	// Max angle
	theta_max = object_theta(ic, jc, label_number, label_image, rgb); //interesting object_theta returns 0.0

	// Beta angles
	b1 = theta_max + (3.14159 / 2); //theta_max + 90deg
	b2 = theta_max + (3.14159 / 4);//theta_max + 45deg
	b3 = theta_max + (3.14159); //theta_max + 0 deg

	// Radius at beta angles

	r1 = object_radius(ic, jc, b1, label_number, label_image, rgb);
	r2 = object_radius(ic, jc, b2, label_number, label_image, rgb);
	r3 = object_radius(ic, jc, b3, label_number, label_image, rgb);

	//maximum radius 
	r_max = object_radius(ic, jc, theta_max, label_number, label_image, rgb);


	// find max radius values
	if (r1>r2 && r1>r3)max = r1;
	else if (r2 > r1 && r2 > r3) max = r2;
	else max = r3;


	//Get min radius values
	if (r1 < r2 && r1 < r3)min = r1;
	else if (r2 < r1 && r2 < r3)min = r2;
	else min = r3;


	//Compare values of hypothetical triangles created by r1,r2,r3
	if ((r_max / min) > 1.414)printf("\n Shape: rectangle ");

	else
	{
		if ((max - min)<3)printf("\n Shape: circle ");

		else printf("\n Shape: probably a square ");
	}




	return 0;
}


void color_detect( int label_number, image &label_image, image &rgb)
{
	
	int size;
	unsigned int Red = 0, Green = 0, Blue = 0;
	unsigned int B, G, R, objSize=1;
	i2byte *pl;

	pl = (i2byte *)label_image.pdata;
	size = rgb.width * rgb.height * 3;
	for (int i = 0; i < size; i += 3)
	{
		if (pl[i / 3] == label_number)
		{
			B = rgb.pdata[i];
			G = rgb.pdata[i + 1];
			R = rgb.pdata[i + 2];

			Red += R;
			Green += G;
			Blue += B;
			objSize++;
		}
	}

	Red /= objSize; Green /= objSize; Blue /= objSize;
	cout << "\n\n\n***The Colors are: R" << Red << " G" << Green << " B" << Blue << "***";
}


int image_pixelcount(image &x, int &count)   //Personal Function
// This function counts the pixels that are 255 on a scaled Grey image
// x must be greyscale image type --- Maybe also RGB, haven't tried that, but it should work
// x - input
// count - output (Added output)
{
	i4byte i, j, size;
	ibyte *px_GREY;
	count = 0;
	

	// check for size compatibility
	if (x.height != 480 || x.width != 640) {
		printf("\nERROR! Incompatible image.");
		return 1;
	}

	// initialize pointers
	px_GREY = x.pdata;

	// number of pixels
	size = (i4byte)x.width * x.height;
	
	for (i = 0; i < size; i++)
	{
		if (px_GREY[i] == 255) 
		{
			count++;
		}
	}
	return 0;
}


int add_image(image &x, image &y, image &z)   //Personal Function
// add/Subtract two images into a third image
// x, y, z must be binary greyscale image types (ie, either 0 or 255 in value)
// x & y - input
// z - output (Added output)
{
	i4byte i, j, size;
	ibyte *px_GREY, *py_GREY, *pz_GREY;

	// check for size compatibility
	if (x.height != y.height || x.width != y.width) {
		printf("\nerror in addition: sizes of the two input images are not the same!");
		return 1;
	}

	else if (z.height != y.height || z.width != y.width) {
		printf("\nerror in addition: sizes of input and output images are not the same!");
		return 1;
	}

	// initialize pointers
	px_GREY = x.pdata;
	py_GREY = y.pdata;
	pz_GREY = z.pdata;

	// number of pixels
	size = (i4byte)x.width * x.height;
	/*
	if (x.type == GREY_IMAGE && y.type == GREY_IMAGE) {
		memcpy((void *)(y.pdata), (void *)(x.pdata), (size_t)size);
	}*/

	for (i = 0, j = 0; i < size; i++, j += 3)
	{
		pz_GREY[i] = abs(px_GREY[i]) + abs(py_GREY[i]);
	    if (pz_GREY[i] > 255) pz_GREY[i] = 255;
		if (pz_GREY[i] < 0) pz_GREY[i] = 0;

	}
	return 0;
}


int subtract_image(image &x, image &y, image &z)   //Personal Function
// add/Subtract two images into a third image
// x, y, z must be greyscale binary image types (ie, either 0 or 255)
// x & y - input
// z - output (Added output)
{
	i4byte i, j, size;
	ibyte *px_GREY, *py_GREY, *pz_GREY;

	// check for size compatibility
	if (x.height != y.height || x.width != y.width) {
		printf("\nerror in subtraction: sizes of the two input images are not the same!");
		return 1;
	}

	else if (z.height != y.height || z.width != y.width) {
		printf("\nerror in subtraction: sizes of input and output images are not the same!");
		return 1;
	}

	// initialize pointers
	px_GREY = x.pdata;
	py_GREY = y.pdata;
	pz_GREY = z.pdata;

	// number of pixels
	size = (i4byte)x.width * x.height;
	/*
	if (x.type == GREY_IMAGE && y.type == GREY_IMAGE) {
		memcpy((void *)(y.pdata), (void *)(x.pdata), (size_t)size);
	}*/

	for (i = 0, j = 0; i < size; i++, j += 3)
	{
		pz_GREY[i] = abs(px_GREY[i]) - abs(py_GREY[i]);
	    if (pz_GREY[i] < 0) pz_GREY[i] = 0; //Check for underflow.
		if (pz_GREY[i] > 255) pz_GREY[i] = 255; //Check for underflow.

	}
	return 0;
}
