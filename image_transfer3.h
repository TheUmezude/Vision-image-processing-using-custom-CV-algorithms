
// image types
// RGB_IMAGE   is 3 bytes per pixel
// GREY_IMAGE  is 1 bytes per pixel
// LABEL_IMAGE is 2 bytes per pixel
#define RGB_IMAGE 1
#define GREY_IMAGE 2
#define LABEL_IMAGE 3 

// define a variable type that holds a single byte of data
typedef unsigned char ibyte;
typedef unsigned short int i2byte;
typedef unsigned long int i4byte;

// define a structure that stores an image
typedef struct {
	int type;   // image type
	i2byte height; // image height
	i2byte width;  // image width
	ibyte *pdata; // pointer to image data
	i2byte nlabels;  // number of labels (used for LABEL_IMAGE type)
} image;

///////

int activate_camera(int cam_number, int height, int width);

int acquire_image(image &a, int cam_number);

int deactivate_cameras();

int stop_camera(int cam_number);

int start_camera(int cam_number);

///////

int view_rgb_image(image &a);

int save_rgb_image(char *file_name, image &a);

int load_rgb_image(char *file_name, image &a);

int allocate_image(image &a);

int free_image(image &a);
