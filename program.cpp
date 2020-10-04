//V3
// included full structure of code
//DISCLAIMER: cap locks does not mean im angry.It is just for clarity :)
//Included shape detection code.

//Include some default libraries
#include "image_transfer3.h"
#include <iostream>
#include <string>
#include <fstream>
#include <stdio.h>
#include <stdlib.h>
#include <conio.h>
#include <cmath>
#include <Windows.h>
#include "timer.h"

//Include image processing libraries
#include "vision.h"

//serial libraries
#include <cstdio>
#include <cstdlib>
#include "serial_com.h"
#include <iostream>
#include <strstream> // for string streams

//Include our team's library
#include "techlead.h"

using namespace std;

#define KEY(c) ( GetAsyncKeyState((int)(c)) & (SHORT)0x8000 ) //Keyboard input macro
double shape_detect(double ic, double jc, int label_number, image &label_image, image &rgb); //For Shape Detection



int main(int argc, char* argv[])
{
	//Variables for the coordinates
	double edges[2][4]; int steps = 40; float objheight = 20;
	double S = 3.2, di = 10, dj = 44;
	ifstream fread; ofstream fwrite;
	


	//NEW for v2 - for scaling factor Sx, and Sy
	double cx1, cx2, cx3; //x coordinates of centroid for label 1,2,3
	double cy1, cy2, cy3; //y coordinates of centroid for label 1, 2, 3
	double dx, dy; //distance between centroids( for 3 point calibration)
	double Sx; //Scaling factor for x
	double Sy;//Scaling factor for y

	//OLD
	//Parameters for Serial Commuication
	HANDLE h1;
	const int NMAX = 64; //Arduino can only support  64 characters in its serial buffer by default
	char buffer[NMAX];  //Char array for the serial communication
	int n;
	int xc = 100, yc = 100, zc = 30;   //Position of center location
	float x = 100.0, y = 100.0, z = objheight; //Initializing the G-Code parsing variables
	char ch;
	//Serial Comms Initialization
	ostrstream sout(buffer, NMAX);
	open_serial("\\\\.\\COM8", h1); // COM9 for my 3D printer
	Sleep(100); // need to wait for data to come
	show_serial(h1); //Get The Welcome message from printer
	Sleep(100);


	//Image Processing Parameters
	//DEFINE  MOAR VARIABLES(image processing)-----------------------------------------------------------------------------------------------
	//CAMERA - Edge Detection
	int nhist, j, nlabels;
	double hist[255], hmin, hmax, ic, jc;
	FILE *fp;
	image a, b, rgb, rgb0; // declare some image structures
	image label;
	int cam_number, height, width;
	double R_ave, G_ave, B_ave;
	//Vision Initialization
	cam_number = 0; //set camera number (normally 0 or 1)
	width = 640;
	height = 480;
	activate_camera(cam_number, height, width);	// activate camera
	rgb.type = RGB_IMAGE;
	rgb.width = width;
	rgb.height = height;
	rgb0.type = RGB_IMAGE;
	rgb0.width = width;
	rgb0.height = height;
	// set the type and size of the images
	a.type = GREY_IMAGE;
	a.width = width;
	a.height = height;
	b.type = GREY_IMAGE;
	b.width = width;
	b.height = height;
	label.type = LABEL_IMAGE;
	label.width = width;
	label.height = height;
	// allocate memory for the images
	allocate_image(a);
	allocate_image(b);
	allocate_image(label);
	allocate_image(rgb);
	allocate_image(rgb0);

	//overhead
	int activity = 1; char calib = n;
	fread.open("calibration_parameters.csv");
	fread >> S >> di >> dj;
	fread.close();

	while (1)
	{
		cout << "\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\nS = " << S << " di = " << di << " dj = " << dj << endl;
		cout << "Change the Calibration parameters? y/n:...\t";
		cin >> calib;

		if (calib=='y')
		{
			cout << "Enter Calibration parameters: S = " << S << " di = " << di << " dj = "<< dj << endl;
			cin >> S >> di >> dj;
			fwrite.open("calibration_parameters.csv");
			fwrite << S << " " << di << " " << dj;
			fwrite.close();

		}

		cout << "\n\n\n\n***************PROGRAM MENU***************" << "\n\n\n";
		cout << "Press any of the below numbers to choose an option" << endl;
		cout << "1. Home the Printer.\n" << endl;
		cout << "2. Detect Centroid and parse G-Code.\n" << endl;
		cout << "3. Detect 4 Edge points and parse G-Code.\n" << endl;
		cout << "4. Detect Centroid, Color, and shape of object.\n" << endl;
		cout << "5. Close the Program.\n" << endl;
		cout << "6. Perform the 9 point Auto Levelling operation.\n" << endl;
		cout << "9. Controll the Printer Head with the keyboard.\n" << endl;
		cout << "What Shall be Done?:...";
		cin >> activity;


		if (activity == 1)
		{
			cout << "Homing The printer" << endl;
			cout << "sending the G28" << endl;
			Sleep(1000);
			//Homing the printer
			// start home ///////////////////
			sout << "G28";  //This homes the printer
			sout << "\n";
			sout << '\0'; // terminate the string so strlen can function
			n = strlen(buffer);
			(n > 64) ? printf("Command Overload!!!!!!") : serial_send(buffer, n, h1); //send the command(s) to the 3D printer / Arduino
			Sleep(200);
			show_serial(h1);//Print Response from arduino
			Sleep(100);
			sout.seekp(0);
		}


		//***************************************************************************************************************************************************
		if (activity == 2)
		{
			sout.seekp(0);

			sout << "G0"; // move without extrusion
			sout << " X" << -33;
			sout << " Y" << 0;
			z = objheight;
			sout << " Z" << z;
			sout << "\n";

			sout << '\0';

			n = strlen(buffer);
			cout << "\nn = " << n;
			cout << "\nbuffer = " << buffer;
			serial_send(buffer, n, h1);
			Sleep(100);

			cout << "Wait for the build-plate to get to X0, Y0, then press a key" << endl;
			cout << "Press 'P' after the first centroid has been detected, to move to the next centroid point\n";
			getch();

			cout << "Determining all Centroids and Dectecting the Shape" << endl;
			acquire_image(rgb, cam_number);
			// copy the original rgb image to use later
			//PROCESS IMAGE(dilate, find centroids ic and jc)-----------------------------------------------------------------------------------------------
			copy(rgb, rgb0);
			copy(rgb, a);
			copy(a, rgb);// convert to RGB image format
			scale(a, b); // scale the image to enhance contrast
			copy(b, a); // put result back into image a
			copy(a, rgb);    // convert to RGB image format
			lowpass_filter(a, b);// apply a filter to reduce noise
			copy(b, a);

			copy(a, rgb);    // convert to RGB image format
			threshold(a, b, 52); // use threshold function to make a binary image (0,255)
			copy(b, a);
			copy(a, rgb);    // convert to RGB image format
			invert(a, b); //invert the image
			copy(b, a);
			copy(a, rgb);    // convert to RGB image format
			erode(a, b); // perform an erosion function to remove noise (small objects)
			copy(b, a);
			
			copy(a, rgb);// convert to RGB image format
			// perform a dialation function to fill in 
			// and grow the objects
			dialate(a, b);
			copy(b, a);
			dialate(a, b);
			copy(b, a);
			copy(a, rgb);    // convert to RGB image format
			label_image(a, label, nlabels);
			//Centroid Determination
			for (int k = 1; k <= nlabels; k++)
			{
				if (KEY('L')) break;
				
				centroid(a, label, k, ic, jc);
				printf("\n\n\nCentroid Value for %d-th label is :(%f, %f)", k, ic, jc);
				//Draw the centroid on the image(optional)
				draw_point(a, ic, jc, 140);
				copy(a, rgb);
				view_rgb_image(rgb);
				//shape_detect(ic, jc, k, label, rgb);
				//object_theta(ic, jc, k, label, rgb);
				view_rgb_image(rgb);

				// set sout position to beginning so we can use it again
				sout.seekp(0);

				x = (ic / S) + di;
				y = (jc / S) + dj;
				z = objheight;

				while (1)
				{
					if (KEY('P')) break;
					if (z < 210)
					{
						if (KEY('W')) z += 2;
						if (z > 5)
						{
							if (KEY('S')) z -= 2;
						}
					}

					sout.seekp(0);
					sout << "G0"; // move without extrusion
					sout << " X" << x;
					sout << " Y" << y;
					sout << " Z" << z;
					sout << "\n";

					sout << '\0'; // terminate the string so strlen can function
					// note the C++ compiler seems to hate "\0"

					n = strlen(buffer); // number of bytes to send (excludes \0)

					// for debugging
					cout << "\nn = " << n;
					cout << "\nbuffer = " << buffer;

					// send the move command to the 3D printer
					serial_send(buffer, n, h1);

					// wait 100 ms for a message to be sent back
					// why wait -> give some time for the move to finish
					// also time for the anet arduino board to process
					// the command --> ie we don't want to fill up
					// its incoming message mailbox
					// note the approximate time to move = dist / speed
					// -> maybe use vision system to see when it stops
					// or gets there
					// -- note there is also a G-code command
					// which sends back the current position
					// -> could use that
					// maybe use istrstream ?
					Sleep(100);

					cout << "\nincoming G-code commands:\n\n";
				}


				while (1)
				{
					if (KEY('P')) break;
					if (z < 210)
					{
						if (KEY('W')) z += 2;
						if (z > 5)
						{
							if (KEY('S')) z -= 2;
						}
					}

					sout.seekp(0);
					sout << "G0"; // move without extrusion
					sout << " X" << x;
					sout << " Y" << y;
					sout << " Z" << z;
					sout << "\n";

					sout << '\0'; // terminate the string so strlen can function
					// note the C++ compiler seems to hate "\0"

					n = strlen(buffer); // number of bytes to send (excludes \0)

					// for debugging
					cout << "\nn = " << n;
					cout << "\nbuffer = " << buffer;

					// send the move command to the 3D printer
					serial_send(buffer, n, h1);

					// wait 100 ms for a message to be sent back
					// why wait -> give some time for the move to finish
					// also time for the anet arduino board to process
					// the command --> ie we don't want to fill up
					// its incoming message mailbox
					// note the approximate time to move = dist / speed
					// -> maybe use vision system to see when it stops
					// or gets there
					// -- note there is also a G-code command
					// which sends back the current position
					// -> could use that
					// maybe use istrstream ?
					Sleep(100);

					cout << "\nincoming G-code commands:\n\n";
				}
				getch();
			}
		}


		//*******************************************************************************************************************************************
		if (activity == 3)
		{
			sout.seekp(0);

			sout << "G0"; // move without extrusion
			sout << " X" << -33;
			sout << " Y" << 0;
			z = objheight;
			sout << " Z" << z;
			sout << "\n";

			sout << '\0';

			n = strlen(buffer);
			cout << "\nn = " << n;
			cout << "\nbuffer = " << buffer;
			serial_send(buffer, n, h1);
			Sleep(100);

			cout << "Wait for the build-plate to get to X0, Y0, then press a key" << endl;
			cout << "Press 'P' after the first edge has been detected, to move to the next edge point\n";
			getch();


			cout << "Detecting the edge points" << endl;
			acquire_image(rgb, cam_number);
			// copy the original rgb image to use later
			//PROCESS IMAGE(dilate, find centroids ic and jc)-----------------------------------------------------------------------------------------------
			copy(rgb, rgb0);
			copy(rgb, a);
			copy(a, rgb);// convert to RGB image format
			scale(a, b); // scale the image to enhance contrast
			copy(b, a); // put result back into image a
			copy(a, rgb);    // convert to RGB image format
			lowpass_filter(a, b);// apply a filter to reduce noise
			copy(b, a);
			copy(a, rgb);    // convert to RGB image format
			threshold(a, b, 70); // use threshold function to make a binary image (0,255)
			copy(b, a);
			copy(a, rgb);    // convert to RGB image format
			invert(a, b); //invert the image
			copy(b, a);
			copy(a, rgb);    // convert to RGB image format
			erode(a, b); // perform an erosion function to remove noise (small objects)
			copy(b, a);
			//Corrector 1
			erode(a, b); // perform an erosion function to remove noise (small objects)
			copy(b, a);
			erode(a, b); // perform an erosion function to remove noise (small objects)
			copy(b, a);
			erode(a, b); // perform an erosion function to remove noise (small objects)
			copy(b, a);
			erode(a, b); // perform an erosion function to remove noise (small objects)
			copy(b, a);
			//Corrector 1
			copy(a, rgb);// convert to RGB image format
			// perform a dialation function to fill in 
			// and grow the objects
			dialate(a, b);
			copy(b, a);
			dialate(a, b);
			copy(b, a);
			copy(a, rgb);    // convert to RGB image format
			label_image(a, label, nlabels);

			for (int k = 1; k <= nlabels; k++)
			{
				centroid(a, label, k, ic, jc);
				x = (ic / S) + di;
				y = (jc / S) + dj;
				z = objheight;
				draw_point_RGB(rgb, ic, jc, 255, 0, 0);
				view_rgb_image(rgb);
				
				while (1)
				{
					cout << "You can adjyust the height of the printer. Press C to continue" << endl;
					if (KEY('C')) break;
					if (z < 210)
					{
						if (KEY('W')) z += 2;
						if (z > 5)
						{
							if (KEY('S')) z -= 2;
						}
					}
					sout.seekp(0);
					sout << "G0"; // move without extrusion
					sout << " X" << x;
					sout << " Y" << y;
					sout << " Z" << z;
					sout << "\n";
					sout << '\0'; 
					n = strlen(buffer);
					cout << "\nn = " << n;
					cout << "\nbuffer = " << buffer;
					serial_send(buffer, n, h1);
					Sleep(100);
					cout << "\nincoming G-code commands:\n\n";
				}

				double r = 0;
				double theta;
				for (int i = 0; i < steps; i++)
				{
					if (KEY('L')) break;
					Sleep(800);
					theta = 2*(i + 1)*3.141593 /steps;
					r = object_radius(ic, jc, theta, k, label, rgb);
					edges[1][i] = (int)(ic + r*cos(theta));
					edges[2][i] = (int)(jc + r*sin(theta));
					draw_point_RGB(rgb, edges[1][i], edges[2][i], 255, 0, 0);
					// set sout position to beginning so we can use it again
					sout.seekp(0);
					x = (edges[1][i] / S) + di;
					y = (edges[2][i] / S) + dj;
					view_rgb_image(rgb);

					sout.seekp(0);
					sout << "G0"; // move without extrusion
					sout << " X" << x;
					sout << " Y" << y;
					sout << " Z" << z;
					sout << "\n";
					sout << '\0'; 
					n = strlen(buffer);
					cout << "\nn = " << n;
					cout << "\nbuffer = " << buffer;
					serial_send(buffer, n, h1);
					Sleep(5);
					cout << "\nincoming G-code commands:\n\n";
				}
				
			}
		}


		//*******************************************************************************************************************************************************
		if (activity == 4)
		{
			sout.seekp(0);

			sout << "G0"; // move without extrusion
			sout << " X" << -33;
			sout << " Y" << 0;
			z = objheight;
			sout << " Z" << z;
			sout << "\n";

			sout << '\0';

			n = strlen(buffer);
			cout << "\nn = " << n;
			cout << "\nbuffer = " << buffer;
			//serial_send(buffer, n, h1);
			Sleep(100);

			cout << "Wait for the build-plate to get to X0, Y0, then press a key" << endl;
			getch();

			cout << "Determining all Centroids, Color, and Dectecting the Shape" << endl;
			acquire_image(rgb, cam_number);
			// copy the original rgb image to use later
			//PROCESS IMAGE(dilate, find centroids ic and jc)-----------------------------------------------------------------------------------------------
			copy(rgb, rgb0);
			copy(rgb, a);
			copy(a, rgb);// convert to RGB image format
			scale(a, b); // scale the image to enhance contrast
			copy(b, a); // put result back into image a
			copy(a, rgb);    // convert to RGB image format
			lowpass_filter(a, b);// apply a filter to reduce noise
			copy(b, a);
			copy(a, rgb);    // convert to RGB image format
			threshold(a, b, 52); // use threshold function to make a binary image (0,255)
			copy(b, a);
			copy(a, rgb);    // convert to RGB image format
			invert(a, b); //invert the image
			copy(b, a);
			copy(a, rgb);    // convert to RGB image format
			erode(a, b); // perform an erosion function to remove noise (small objects)
			copy(b, a);
			copy(a, rgb);// convert to RGB image format
			// perform a dialation function to fill in 
			// and grow the objects
			dialate(a, b);
			copy(b, a);
			dialate(a, b);
			copy(b, a);
			copy(a, rgb);    // convert to RGB image format
			label_image(a, label, nlabels);

			for (int k = 1; k <= nlabels; k++)
			{
				if (KEY('L')) break;
				centroid(a, label, k, ic, jc);
				printf("\n\n\nCentroid Value for %d-th label is :(%f, %f)", k, ic, jc);
				//Draw the centroid on the image(optional)
				draw_point(a, ic, jc, 140);
				copy(a, rgb);
				view_rgb_image(rgb);
				shape_detect(ic, jc, k, label, rgb);
				object_theta(ic, jc, k, label, rgb);
				view_rgb_image(rgb);
				color_detect(k, label, rgb0);
			}
		}


		//********************************************************************************************************************************
		if (activity == 5)
		{
			free_image(a);
			free_image(b);
			free_image(label);
			free_image(rgb);
			free_image(rgb0);
			deactivate_cameras();
			close_serial(h1);
			printf("\n\ndone.\n");
			getch();
			return 0; // no errors
		}


		if (activity == 6)
		{
			cout << "Performing the 9 point Auto Levelling Operation.\nPress a key Once the operation is done" << endl;
			sout.seekp(0);

			sout << "G29"; // move without extrusion
			sout << "\n";
			sout << '\0';
			n = strlen(buffer);
			cout << "\nn = " << n;
			cout << "\nbuffer = " << buffer;
			serial_send(buffer, n, h1);
			Sleep(100);
			getch();
		}

		if (activity == 9)
		{
			sout.seekp(0);
			sout << "G28";
			sout << "\n";
			sout << '\0';
			n = strlen(buffer);
			serial_send(buffer, n, h1);
			Sleep(4000);
			
			sout.seekp(0);
			sout << "G0"; // move without extrusion
			sout << " X" << 100;
			sout << " Y" << 100;
			z = objheight;
			sout << " Z" << z;
			sout << "\n";
			sout << '\0';

			n = strlen(buffer);
			cout << "\nn = " << n;
			cout << "\nbuffer = " << buffer;
			serial_send(buffer, n, h1);
			Sleep(100);

			while (1)
			{
				if (KEY('P')) break;
				if (y < 210)
				{
					if (KEY(VK_UP)) y += 2;
					if (y > 5)
					{
						if (KEY(VK_DOWN)) y -= 2;
					}
				}

				if (x < 210)
				{
					if (KEY(VK_RIGHT)) x += 2;
					if (x > 5)
					{
						if (KEY(VK_LEFT)) x -= 2;
					}
				}

				if (z < 210)
				{
					if (KEY('W')) z += 2;
					if (z > 5)
					{
						if (KEY('S')) z -= 2;
					}
				}

				sout.seekp(0);
				sout << "G0"; // move without extrusion
				sout << " X" << x;
				sout << " Y" << y;
				sout << " Z" << z;
				sout << "\n";

				sout << '\0'; // terminate the string so strlen can function
				// note the C++ compiler seems to hate "\0"

				n = strlen(buffer); // number of bytes to send (excludes \0)

				// for debugging
				cout << "\nn = " << n;
				cout << "\nbuffer = " << buffer;

				// send the move command to the 3D printer
				serial_send(buffer, n, h1);
				Sleep(100);
				cout << "\nincoming G-code commands:\n\n";
			}
		}

	}
}



































































/*cout << "scaling factor determination";
// Move Bed to Calibration position ///////////////////
sout.seekp(0);
sout << "G0 X0 Y0\n";  //This homes the printer
sout << '\0'; // terminate the string so strlen can function
n = strlen(buffer);
(n > 64) ? printf("Command Overload!!!!!!") : serial_send(buffer, n, h1); //send the command(s) to the 3D printer / Arduino
Sleep(200);
show_serial(h1);//Print Response from arduino
Sleep(100);
cout << buffer;
*/

//SCALING FACTOR CALCULATIONS( Check the code v6, inside of Vivek's folder. We'll run it seperatly. )-------------------------------------------------------------
/*
centroid(a, label, 1, cx1, cy1);
printf("\n\nThe centroid value for label UNO is (%f,%f)", cx1, cy1);


//Calculate centroid coordinates for cx1,cy1
centroid(a, label, 2, cx2, cy2);
printf("\nThe centroid value for label DOS is (%f,%f)", cx2, cy2);

//Calculate centroid coordinates for cx1, cy1
centroid(a, label, 3, cx3, cy3);
printf("\nThe centroid value for label TREIZE is (%f,%f)", cx3, cy3);


//Calculate dx, dy
//Label infor
//Label 1: circle
//Label 2: square
//Label 3: pen
dx = abs(cx1 - cx2);
dy = abs(cy2 - cy3);
printf("\ndx = %f, dy = %f ", dx, dy);

//Calculate Scaling factor
//Sx = [insert value in mm]/dx;
//Sy = [insert value in mm] / dy;
//printf("\nScaling factor Sx = %f ",Sx);
//printf("\nScaling factor SY = %f ", Sy);

*/