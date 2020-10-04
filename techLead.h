//The functions in techLead.cpp, and techLead.h were all created by members of the group

#ifndef TECH_H
#define TECH_H

int draw_point_RGB(image &rgb, int ip, int jp, int R, int G, int B);

double object_radius(double ic, double jc, double beta, int label_number, image &label_image, image &rgb);

double object_theta(double ic, double jc, int label_number, image &label_image, image &rgb);

double shape_detect(double ic, double jc, int label_number, image &label_image, image &rgb);

void color_detect(int label, image &label_image, image &rgb);

int image_pixelcount(image &x, int &count); //For calculating Area.

int add_image(image &x, image &y, image &z); //For adding binary images together.

int subtract_image(image &x, image &y, image &z); //For subtracting binary images from eachother.


/*
Subtract_image, add_image, image_pixel count were used for motion detection operations
*/

#endif