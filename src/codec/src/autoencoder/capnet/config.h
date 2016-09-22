#include <libpng12/png.h>
#include <gif_lib.h>
#include <setjmp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <math.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <arpa/inet.h>

#define BIAS   -1.0 /* activation value of the bias neuron              */
#define NUM_I  2    /* number of input neurons - w/o bias               */
#define NUM_H  3    /* number of hidden neurons - w/o bias              */
#define NUM_O  1    /* number of output neurons                         */
#define N_IMGS 1    /* number of patterns in the training set           */
#define EPOCHS 1000 /* number of times to cycle through the example set */

#define BASE_DIR  "/tmp/capnet/"

typedef enum {
  IMG_PNG=1, IMG_GIF
} img_t;

/* record information for PNG image file */
struct png_d {
  unsigned char *image;
  int width;
  int height;
  int bit_depth;
  int channels;
};

double *sumsh,*sumso;  // stores the weighted sums for each layer of neurons
double **wih, **who;   // stores the weights for each layer

// capnet.c
int trainNet(double *, double *);
int BackProp(int, int, double *, double *);
double **initWeights(int, int);
double sigmoid(double);
double sigmoidPrime(double);
void FeedForward(unsigned char *, int, int, int, int, double *, double *);

// img.c
struct png_d *readpngtobitmap(char *filename);
unsigned char *readgiftobitmap(char *filename, int *width, int *height);
void *bmalloc(size_t size);
void *xmalloc(size_t size);