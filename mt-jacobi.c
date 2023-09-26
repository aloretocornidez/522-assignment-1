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
#include <sys/time.h>
// #include <sys/types.h>

#define TOLERANCE 0.001

/* Function Prototpyes */
void InitializeGrids(); // Initizalizes the grids used jacobian calculation.
double **AllocateGrid(int, int); // Allocates grid memory.
void jacobi(int threadId);       // Calculates the jacobi.
void *worker(void *arg);
double Elapsed(struct timeval end,
               struct timeval start); // Keeps track of the time elapsed.

/* Varaible Declarations */
int numThreads;
int gridSize, numIters;

double maxDiff;
double **grid1, **grid2;

int main(int argc, char *argv[]) {

  printf("Here");

  int *params;
  pthread_t *threads;        // Thread Handles
  struct timeval start, end; // Time elapsed.

  // Read command line and initialize grids
  gridSize = atoi(argv[1]);
  numIters = atoi(argv[2]);
  numThreads = atoi(argv[3]);

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

  // Get starting time.
  gettimeofday(&start, NULL);

  // Create threads and execute the worker function.
  for (int i = 0; i < numThreads; i++) {
    params[i] = i;
    pthread_create(&threads[i], NULL, worker, (void *)(&params[i]));
  }

  // Wait until all threads are finished.
  for (int i = 0; i < numThreads; i++) {
    pthread_join(threads[i], NULL);
  }

  // Get ending Time.
  gettimeofday(&end, NULL);

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

  int done = 0;
  double temp;
  double maxdiff;
  int iters = 0;

  // compute bounds for this threads---just algebra
  int startRow = myId * gridSize / numThreads;
  int endRow = (myId + 1) * (gridSize / numThreads) - 1;

  // Currently, this is how it is done in the sequential version of the program.

  while (!done) {
    /* update my points */
    for (i = startRow; i <= endRow; i++) {
      for (j = 1; j <= gridSize; j++) {

        grid2[i][j] = (grid1[i - 1][j] + grid1[i + 1][j] + grid1[i][j - 1] +
                       grid1[i][j + 1]) *
                      0.25;
      }
    }

    // Resetting the maximum difference.
    maxdiff = 0.0;

    // Update my points again and find the max difference between any two
    // points.
    for (i = startRow; i <= endRow; i++) {
      for (j = 1; j <= gridSize; j++) {

        grid1[i][j] = (grid2[i - 1][j] + grid2[i + 1][j] + grid2[i][j - 1] +
                       grid2[i][j + 1]) *
                      0.25;

        // Since both grids are calculated at this point,
        // the difference between both grids can be calculated.
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
    if (maxdiff < TOLERANCE || iters >= numIters) {
      done = 1;
    }


  }

  maxDiff = maxdiff;
  return;
}

// Wrapper funciton to allod threads to be allocated.
void *worker(void *arg) {
  // Dereference the value inside the function.
  int id = *((int *)arg);
  jacobi(id);
  return NULL;
}

double Elapsed(struct timeval end, struct timeval start) {
  return ((end.tv_sec + end.tv_usec * 0.000001) -
          (start.tv_sec + start.tv_usec * 0.000001));
}

double **AllocateGrid(int N, int M) {
  int i;
  double *vals, **temp;

  // allocate values
  vals = (double *)malloc(N * M * sizeof(double));

  // allocate vector of pointers
  temp = (double **)malloc(N * sizeof(double *));

  for (i = 0; i < N; i++)
    temp[i] = &(vals[i * M]);

  return temp;
}
