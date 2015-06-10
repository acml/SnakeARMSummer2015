#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>



typedef struct {
  int width, height;
  int nChannels;
  int widthStep;
  int depth;
  uint8_t *pixelsData;
} image_t;

enum{ BUFFER_SIZE = 16};

typedef enum{
  IMG_OK,
  IMG_OPEN_FAILURE,
  IMG_MISSING_FORMAT,
  IMG_INVALID_FORMAT,
  IMG_INSUFFICIENT_MEMORY,
  IMG_INVALID_SIZE,
  IMG_INVALID_DEPTH,
  IMG_READ_FAILURE,
  IMG_WRITE_FAILURE
} image_error_t;

image_error_t image_read(const char *filename, image_t **out);
void printCode(image_t *img, char* img_out_filename);
void image_free(image_t *image);

int main(int argc, char **argv) {
	 image_t *img_in = NULL;
	 char* img_in_filename = "/home/aj3214/Downloads/niEXE66iA.bmp";
	 image_error_t error = image_read(img_in_filename, &img_in);
   char* img_out_filename = argv[1];
	 printCode(img_in, img_out_filename);
	 if(error){
      printf("error");
   }
   image_free(img_in);
   return 0;
}

void image_free(image_t *image) {
  if (image == NULL) {
    return;
  }

  if (image->pixelsData != NULL) {
    free(image->pixelsData);
  }

  free(image);
}

image_error_t image_read(const char *filename, image_t **out) {
  image_t *image;

  FILE *in;
  char buffer[BUFFER_SIZE], c;

  /*
   * Open the file for reading.
   */
  in = fopen(filename, "rb");
  if (!in) {
    return IMG_OPEN_FAILURE;
  }

  /*
   * Read in the image format; P6 and P4 is currently the only format supported.
   */
  if (fgets(buffer, BUFFER_SIZE, in) == NULL) {
    return IMG_MISSING_FORMAT;
  }

  if (buffer[0] != 'P' && ( buffer[1] != '4' || buffer[1] != '6' || buffer[1] == '5' )) {
    return IMG_INVALID_FORMAT;
  }

  /*
   * Allocate space for the image header.
   */
  image = malloc(sizeof(image_t));

  if (image == NULL) {
      return IMG_INSUFFICIENT_MEMORY;
  }

  if( buffer[1] == '4' ||buffer[1] == '5' ){
      image->nChannels = 1;
      image->depth = 255;
      
  }

  c = fgetc(in);
   while (c == '#') {
     while (fgetc(in) != '\n') { }
     c = fgetc(in);
   }

   ungetc(c, in);

  /*
   * Read the image's size information.
   */
  if (fscanf(in, "%d %d", &image->width, &image->height) != 2) {
    return IMG_INVALID_SIZE;
  }
  /*
   * Add the widthstep information to allow to move between array
   */
  image->widthStep = image->nChannels * image->width;
 

  /*
   * Skip over any remaining cruft preceding the pixel data.
   */
  while (fgetc(in) != '\n') { }

  /*
   * Allocate and read pixel data.
   */
  image->pixelsData = malloc(image->width * image->height * image->nChannels * sizeof(uint8_t));
  if (!image->pixelsData) {
    return IMG_INSUFFICIENT_MEMORY;
  }

  if (fread(image->pixelsData, image->nChannels * image->width *sizeof(uint8_t),
        image->height, in) != image->height) {

    image_free(image);
    return IMG_READ_FAILURE;
  }

  fclose(in);
  *out = image;
  return IMG_OK;
}

unsigned long createRGB(int r, int g, int b)
{   
    return ((r & 0xff) << 16) + ((g & 0xff) << 8) + (b & 0xff);
}

void printCode(image_t *img, char* img_out_filename) {
    if(img == NULL){
        return;
    }
    FILE *out;

    /*
     * Open file for output.
     */
    out = fopen(img_out_filename, "wb");
    if (!out) {
        return ;
    }

    int colour = 0;
    int r;
    int g;
    int b;
    /*
     * read the src image. Write helper fun
     */
     fprintf(out, "mov r0, r5\n");
     fprintf(out, "mov r1, r6\n");
     fprintf(out, "\n");
     for(int i=0 ; i < img->height; i++)
        {
         for(int j=0; j < img->width; j++)
            {
                r = img->pixelsData[ i* img->widthStep + j * img->nChannels];

                g = img->pixelsData[ i* img->widthStep 
                + j * img->nChannels +1];

                b = img->pixelsData[ i* img->widthStep
                + j * img->nChannels + 2];

                colour = createRGB(r, g, b);
                fprintf(out, "ldr r2,=0xff%x\n", colour);
                fprintf(out, "bl DrawPixel\n");
                
                fprintf(out, "add r1,#1\n");
                fprintf(out, "\n");
            }
            fprintf(out, "mov r1,r6\n");
            fprintf(out, "add r0,#1\n");
            fprintf(out, "\n");
        }
}