/*
 * Author: Alan Manuel Loreto Cornídez
 * File: mt-jacobian.c
 * Purpose: this file executes the calculation of the Jacobian of a matrix
 * using multiple threads via the use <pthread.h> library.
 * Assignment: CSC 522 Parallel Computing | The University of Arizona
 *
 */
#include <math.h>
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

void allocateBarrierArray(int numThreads); // Allocates the barrier array.
void disseminationBarrier(int myId, int numThreads);

/* Variable Declarations */
int numThreads;        // The number of threads executed.
int numIters;          // The max number of iterations allowed.
volatile int *arrival; // The array used for the dissemination barrier.

// Value Arrays for calcualting jacobian.
double maxDiff;
double **grid1, **grid2;
int gridSize;

int main(int argc, char *argv[]) {

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

  InitializeGrids();

  // Allocate thread handles
  threads = (pthread_t *)malloc(numThreads * sizeof(pthread_t));
  params = (int *)malloc(numThreads * sizeof(int));

  // Get starting time.
  gettimeofday(&start, NULL);

  // Implement the barriers here.
  allocateBarrierArray(numThreads);

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

  int threadNum = atoi(argv[2]);
  printf("%d 0 %.3f %.5f\n", threadNum, Elapsed(end, start), maxDiff);

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
  int i, j;

  int done = 0;
  double temp;
  double maxdiff;
  int iters = 0;

  // compute bounds for this threads---just algebra
  int startRow = myId * gridSize / numThreads;
  int endRow = (myId + 1) * (gridSize / numThreads) - 1;

  // Currently, this is how it is done in the sequential version of the program.

  while (!done) {
    disseminationBarrier(myId, numThreads);

    /* update my points */
    for (i = startRow + 1; i <= endRow; i++) {
      for (j = 1; j <= gridSize; j++) {
        grid2[i][j] = (grid1[i - 1][j] + grid1[i + 1][j] + grid1[i][j - 1] +
                       grid1[i][j + 1]) *
                      0.25;
      }
    }

    // Resetting the maximum difference.
    maxdiff = 0.0;

    /*
     * Barrier is inserted in between the calculation of the
     * new value at each grid point.
     * This is because the new grid value is dependent on old
     * grid values having been calculated.
     *
     * The threads access rows that are not calculated by that
     * thread, so thread2 (wlog) must wait for thread1 to finish
     * calculation of the previous gread values.
     *
     * The same is true for the barrier implemented after the
     * grid1 is calcualted again (the barrier after the
     * second for loop.)
     */
    disseminationBarrier(myId, numThreads);

    // Update points again, find the max difference between any two points.
    for (i = startRow + 1; i <= endRow; i++) {
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

    /*
     * Here is the second barrier, the reasoning for the position of the barrier
     * is stated above.
     *
     */
    // disseminationBarrier(myId, numThreads);

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

void allocateBarrierArray(int numThreads) {
  int *temp;

  // Allocate an array
  temp = (int *)malloc(sizeof(int) * numThreads);

  // Initialzing the array to zeros.
  for (int i = 0; i < numThreads; i++) {
    temp[i] = 0;
  }

  arrival = temp;
}

void disseminationBarrier(int threadId, int threads) {

  int lookAt;

  for (int j = 1; j <= ceil(log2(numThreads)); j++) {

    // Spin
    while (arrival[threadId] != 0) {
      ;
    }

    arrival[threadId] = j;

    // Look at is the value that thread a modifies to allow thread b to
    // continue.
    lookAt = (int)(threadId + pow(2, (j - 1))) % threads;

    // Spin Again.
    while (arrival[lookAt] != j) {
      printf("numThreads: %d Thread Id: %d, lookAt: %d  arrival[lookAt]: %d, j: %d\n", numThreads, threadId, lookAt, arrival[lookAt], j);
    }

    arrival[lookAt] = 0;
  }

  return;
}
