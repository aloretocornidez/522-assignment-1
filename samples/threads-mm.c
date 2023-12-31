// CSc 522
// Matrix multiplication example with pthreads

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>

double **a, **b, **c;
int numThreads, matrixSize;

double **allocateMatrix() {
  int i;
  double *vals, **temp;

  // allocate values
  vals = (double *)malloc(matrixSize * matrixSize * sizeof(double));

  // allocate vector of pointers
  temp = (double **)malloc(matrixSize * sizeof(double *));

  for (i = 0; i < matrixSize; i++)
    temp[i] = &(vals[i * matrixSize]);

  return temp;
}

void printMatrix(double **mat) {
  int i, j;

  printf("The %d * %d matrix is\n", matrixSize, matrixSize);
  for (i = 0; i < matrixSize; i++) {
    for (j = 0; j < matrixSize; j++)
      printf("%lf ", mat[i][j]);
    printf("\n");
  }
}

void mm(int myId) {
  int i, j, k;
  double sum;

  // compute bounds for this threads---just algebra
  int startrow = myId * matrixSize / numThreads;
  int endrow = (myId + 1) * (matrixSize / numThreads) - 1;

  // matrix mult over the strip of rows for this thread
  for (i = startrow; i <= endrow; i++) {
    for (j = 0; j < matrixSize; j++) {
      sum = 0.0;

      // Calculate the value for th point at coordinate [i][]
      for (k = 0; k < matrixSize; k++) {
        sum = sum + a[i][k] * b[k][j];
      }
      // Assign the value outside of the loop to minimize memory acceses.
      c[i][j] = sum;
    }
  }
}

void *worker(void *arg) {
  int id = *((int *)arg);
  mm(id);
  return NULL;
}

int main(int argc, char *argv[]) {
  int i, j;
  int *params;
  pthread_t *threads;

  if (argc != 3) {
    printf("Usage: %s <size> <n>,  where size is dimension of square matrix, "
           "and n is number of threads\n",
           argv[0]);
    exit(1);
  }

  matrixSize = atoi(argv[1]);
  numThreads = atoi(argv[2]);

  a = allocateMatrix();
  b = allocateMatrix();
  c = allocateMatrix();

  for (i = 0; i < matrixSize; i++)
    for (j = 0; j < matrixSize; j++) {
      a[i][j] = i + j;
      b[i][j] = i + j;
    }

  if (matrixSize < 10) { // print matrices if relatively small
    printMatrix(a);
    printMatrix(b);
  }

  // Allocate thread handles
  threads = (pthread_t *)malloc(numThreads * sizeof(pthread_t));
  params = (int *)malloc(numThreads * sizeof(int));

  // Create threads
  for (i = 0; i < numThreads; i++) {
    params[i] = i;
    pthread_create(&threads[i], NULL, worker, (void *)(&params[i]));
  }

  for (i = 0; i < numThreads; i++) {
    pthread_join(threads[i], NULL);
  }

  if (matrixSize < 10) // print matrix if relatively small
    printMatrix(c);

  free(params);
  free(threads);
}
