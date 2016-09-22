/*
 * A neural network for defeating CAPTCHA programs which are designed specifically for
 * differentiating between human and machine.
 */

#include "config.h"

const float lrate = 0.8;

int main(int argc, char **argv)
{  
  double *netout = malloc(sizeof(double) * NUM_O); // what the network thinks
  double *hidout = malloc(sizeof(double) * NUM_H); // results from hidden layer
  sumsh = malloc(sizeof(double) * NUM_H); // weighted summations for hidden layer
  sumso = malloc(sizeof(double) * NUM_O); // weighted summations for output layer
  
  srand(time(NULL)*getpid());
  
  /* initialize the weights; small, random values */
  wih = initWeights(NUM_I, NUM_H); // weights between input and hidden layer
  who = initWeights(NUM_H, NUM_O); // weights between hidden and output layer
  
  /* train this thing. */
  printf("%s\n", trainNet(netout, hidout) ? "Epoch limit exceeded!" : "Neural network trained and ready to go.");
  
  free(sumsh); free(sumso); free(wih); free(who);

  return 0;
}

int trainNet(double *netout, double *hidout)
{
  int i,j;
  struct png_d *pd;
  
  for (i=0; i<EPOCHS; i++) {
    for (j=0; j<N_IMGS; j++) {
      pd = readpngtobitmap("sample.png");
      FeedForward(pd->image, pd->width, pd->height, pd->channels, pd->bit_depth, hidout, netout);
      //BackProp(pd->image, p[j][1], hidout, netout);
      free(pd->image);
      free(pd);
    }
  }
  free(netout); free(hidout);
  return 1;
}

/*
 * Take the input pattern propagate forward to calculate output.
 * Compute the weighted sum for each layer, if we are using the bias weight then use
 * -1.0 for the activation value from the previous layer.
 */
void FeedForward(unsigned char *image, int width, int height, int channels, int bit_depth, double *hidout, double *netout)
{
  int i,j;
  
  // Compute activations over input layer for the hidden neurons
  memset(sumsh, 0, sizeof(double) * NUM_H);
  for (i=0; i<NUM_H; i++) {
    for (j=0; j < width*height+1; j+=3)
      sumsh[i] += (j>=NUM_I ? (BIAS*wih[j][i]) : (((1 << j) & image ? 1.0 : 0.0)*wih[j][i]));
    hidout[i] = sigmoid(sumsh[i]);
  }
  // Compute activations over hidden layer for output neurons
  memset(sumso, 0, sizeof(double) * NUM_O);
  for (i=0; i<NUM_O; i++) {
    for (j=0; j<NUM_H+1; j++)
      sumso[i] += (j>=NUM_H ? BIAS*who[j][i] : hidout[j]*who[j][i]);
    netout[i] = sigmoid(sumso[i]);
  }
}

/*
 *
 *  Back Propagation algorithm:
 *  the output given by the network after forward propagation of the input is compared
 *  with the true value for the input to compute some measure of error. any amount of error
 *  will be potentially caused by any layer in the network, thus the error value will be
 *  propagated backward through the layers and used to update the weights of those layers'
 *  weights.
 *
 */
int BackProp(int pattern, int target, double *hidout, double *netout)
{
  int i,j,k;
  double *err = malloc(sizeof(double) * NUM_O);   // stores the errors for the output layer
  double *deltao = malloc(sizeof(double) * NUM_O);// stores the deltas for the output layer
  double *deltah = malloc(sizeof(double) * NUM_H);// stores the deltas for the hidden layer
  
  memset(deltao, 0, sizeof(double) * NUM_O);
  memset(deltah, 0, sizeof(double) * NUM_H);
  
  // Compute hidden to output layer deltas
  for (i=0; i<NUM_O; i++) {
    err[i] = target - netout[i];
    printf("%d, %d - %f = err=%f\n",pattern,target,netout[i],err[i]);
    deltao[i] = err[i] * sigmoidPrime(sumso[i]);
  }
  // Compute input to hidden layer deltas
  for (i=0; i<NUM_H; i++) {
    for (j=0; j<NUM_O; j++)
      deltah[i] += sigmoidPrime(sumsh[i])*who[i][j]*deltao[j];
  }
  // Update who
  for (i=0; i<NUM_O; i++) {
    for (j=0; j<NUM_H+1; j++)
      who[j][i] += (j>=NUM_H ? lrate*BIAS*deltao[i] : lrate*hidout[j]*deltao[i]);
  }
  // Update wih
  for (j=0; j<NUM_H; j++) {
    for (k=0; k<NUM_I+1; k++)
      wih[k][j] += (k>=NUM_I ? lrate*BIAS*deltah[j] : lrate*((1 << k) & pattern ? 1 : 0)*deltah[j]);
  }
  // Clean up
  free(err); free(deltao); free(deltah);
  return 1;
}

double sigmoid(double x)
{
  return 1.0/(1.0 + exp(-1.0*x));
}

double sigmoidPrime(double x)
{
  return sigmoid(x)*(1 - sigmoid(x));
}

/*
 *  Initialize weights to small, random values between -1.0 and 1.0.
 *   - organized into "groups" by feeding layer, n1
 */
double **initWeights(int n1, int n2)
{
  int i,j;
  double **weights = malloc(sizeof(double *) * (n1+1)); // include an extra for the bias neuron
  
  for (i=0; i<n1+1; i++) {
    weights[i] = malloc(sizeof(double) * n2);
    for (j=0; j<n2; j++)
      weights[i][j] = (random() % 2 ? (double)random() : -1.0*(double)random())/(double)RAND_MAX;
  }
  return weights;
}