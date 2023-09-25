/*
 * Author: Alan Manuel Loreto Cornídez
 * File: mt-jacobian.c
 * Purpose: this file executes the calcualtion of the Jacobian of a matrix
 * using multiple threads via the use <pthread.h> library.
 * Assignment: CSC 522 Parallel Computing | The University of Arizona
 */
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>

#define TOLERANCE 0.001

/* Function Prototpyes */
void InitializeGrids(); // Initizalizes the grids used jacobian calculation.
double **AllocateGrid(int, int); // Allocates grid memory.
void jacobi(int threadId);       // Calculates the jacobi.
void *worker(void *arg);

/* Varaible Declarations */
int numThreads, matrixSize;
int gridSize, numIters;

double maxDiff;
double **grid1, **grid2;

int main(int argc, char *argv[]) {
  int i, j;
  int *params;
  pthread_t *threads;

  double maxdiff = 0.0;
  struct timeval start, end;

  // Error if the entries are wrong.
  if (argc != 3) {
    printf("Usage: %s <size> <n>,  where size is dimension of square matrix, "
           "and n is number of threads\n",
           argv[0]);
    exit(1);
  }

  // Read command line and initialize grids
  gridSize = atoi(argv[1]);
  numIters = atoi(argv[2]);

  // AllocateGrids that are used for a function.
  grid1 = AllocateGrid(gridSize + 2, gridSize + 2);
  grid2 = AllocateGrid(gridSize + 2, gridSize + 2);

  /* read command line and initialize grids */
  gridSize = atoi(argv[1]);
  numIters = atoi(argv[2]);

  grid1 = AllocateGrid(gridSize + 2, gridSize + 2);
  grid2 = AllocateGrid(gridSize + 2, gridSize + 2);

  InitializeGrids();

  // Allocate thread handles
  threads = (pthread_t *)malloc(numThreads * sizeof(pthread_t));
  params = (int *)malloc(numThreads * sizeof(int));

  // Create threads and execute the worker function.
  for (i = 0; i < numThreads; i++) {
    params[i] = i;
    pthread_create(&threads[i], NULL, worker, (void *)(&params[i]));
  }

  // Wait until all threads are finished.
  for (i = 0; i < numThreads; i++) {
    pthread_join(threads[i], NULL);
  }

  // Free the memory to finish off.
  free(params);
  free(threads);
}

// InitializeGrids
void InitializeGrids() {
  int i, j;

  for (i = 1; i <= gridSize; i++)
    for (j = 1; j <= gridSize; j++) {
      grid1[i][j] = 0.0;
    }
  for (i = 0; i <= gridSize + 1; i++) {
    grid1[i][0] = 1.0;
    grid1[i][gridSize + 1] = 1.0;
    grid2[i][0] = 1.0;
    grid2[i][gridSize + 1] = 1.0;
  }
  for (j = 0; j <= gridSize + 1; j++) {
    grid1[0][j] = 1.0;
    grid2[0][j] = 1.0;
    grid1[gridSize + 1][j] = 1.0;
    grid2[gridSize + 1][j] = 1.0;
  }
}

// jacobi
void jacobi(int myId) {
  int i, j, k;

  double sum;
  int done = 0;
  double temp;
  double maxdiff;
  int iters;

  // compute bounds for this threads---just algebra
  int startrow = myId * matrixSize / numThreads;
  int endrow = (myId + 1) * (matrixSize / numThreads) - 1;

  // Currently, this is how it is done in the sequential version of the program.

  while (!done) {
    /* update my points */
    for (i = 1; i <= gridSize; i++) {
      for (j = 1; j <= gridSize; j++) {

        grid2[i][j] = (grid1[i - 1][j] + grid1[i + 1][j] + grid1[i][j - 1] +
                       grid1[i][j + 1]) *
                      0.25;
      }
    }
    maxdiff = 0.0;
    // update my points again and find the max difference between any two points
    grid2[i][j] = (grid1[i - 1][j] + grid1[i + 1][j] + grid1[i][j - 1] +
                   grid1[i][j + 1]) *
                  0.25;
    grid1[i][j] = (grid2[i - 1][j] + grid2[i + 1][j] + grid2[i][j - 1] +
                   grid2[i][j + 1]) *
                  0.25;

    //
    for (i = 1; i <= gridSize; i++) {
      for (j = 1; j <= gridSize; j++) {
        grid1[i][j] = (grid2[i - 1][j] + grid2[i + 1][j] + grid2[i][j - 1] +
                       grid2[i][j + 1]) *
                      0.25;

        temp = grid1[i][j] - grid2[i][j];
        if (temp < 0)
          temp = -temp;
        if (maxdiff < temp)
          maxdiff = temp;
      }
    }

    // Increments the number of iterations.
    iters++;

    // Breaks out of the loops after the maxdiff is below
    // the tolerance of the max nnumber of iterations is reached.

    done = 1;
  }

  maxDiff = maxdiff;
  return;
}

// worker
void *worker(void *arg) {
  int id = *((int *)arg);
  jacobi(id);
  return NULL;
}
