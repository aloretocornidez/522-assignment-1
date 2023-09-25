// CSc 522
// MPI+pthreads matrix multiplication example
// Assumes that number of MPI ranks divides N

#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#define TAG 13

double **A, **B, **C;
int stripSize, numThreads, N;

void mm(int myId) {
  int i,j,k;
  double sum;

  // compute bounds for this threads---just algebra
  int startrow = myId * stripSize/numThreads;
  int endrow = (myId+1) * (stripSize/numThreads) - 1;

  // matrix mult over the strip of rows for this thread
  for (i = startrow; i <= endrow; i++) {
    for (j = 0; j < N; j++) {
      sum = 0.0;
      for (k = 0; k < N; k++) {
        sum = sum + A[i][k] * B[k][j];
      }
      C[i][j] = sum;
    }
  }
}

void *worker(void *arg)
{
  int id = *((int *) arg);
  mm(id);
  return NULL;
}

double **allocateMatrix(int N, int M) {
  int i;
  double *vals, **temp;

  // allocate values
  vals = (double *) malloc (N * M * sizeof(double));

  // allocate vector of pointers
  temp = (double **) malloc (N * sizeof(double*));

  for(i=0; i < N; i++)
    temp[i] = &(vals[i * M]);

  return temp;
}

void printMatrix(double **mat, int N, int M) {
  int i,j;

  printf("The %d * %d matrix is\n", N, M);
  for(i=0; i < N; i++){
    for(j=0; j < M; j++)
      printf("%lf ",  mat[i][j]);
    printf("\n");
  }
}

int main(int argc, char *argv[]) {
  double startTime, endTime, sum;
  int numElements, offset, myId, numProcesses, i, j, k, provided;
  int *params;
  pthread_t *threads;

  MPI_Init_thread(&argc, &argv, MPI_THREAD_FUNNELED, &provided);
  
  MPI_Comm_rank(MPI_COMM_WORLD, &myId);
  MPI_Comm_size(MPI_COMM_WORLD, &numProcesses);
  
  N = atoi(argv[1]);
  numThreads = atoi(argv[2]);

  // allocate A, B, and C --- note that you want these to be
  // contiguously allocated.  Workers need less memory allocated.
  
  if (myId == 0) {
    A = allocateMatrix(N, N);
    C = allocateMatrix(N, N);
  }
  else {
    A = allocateMatrix(N/numProcesses, N);
    C = allocateMatrix(N/numProcesses, N);
  }
  
  B = allocateMatrix(N, N);
  
  if (myId == 0) {
    // initialize A and B
    for (i=0; i<N; i++) {
      for (j=0; j<N; j++) {
        A[i][j] = i+j;
        B[i][j] = i+j;
      }
    }
  }

  // print out matrices here, if I'm the administrator
  if (myId == 0 && N < 10) {
    printMatrix(A, N, N);
    printMatrix(B, N, N);
  }
  
  // start timer
  if (myId == 0) {
    startTime = MPI_Wtime();
  }
  
  stripSize = N/numProcesses;

  MPI_Scatter(A[0], stripSize * N, MPI_DOUBLE, A[0], stripSize * N, MPI_DOUBLE, 0, MPI_COMM_WORLD);

#if 0
  // send each process its piece of A -- done via MPI_Scatter in this example
  // left here (but commented out) to make clear what MPI_Scatter is doing
  if (myId == 0) {
    offset = stripSize;
    numElements = stripSize * N;
    for (i=1; i<numProcesses; i++) {
      MPI_Send(A[offset], numElements, MPI_DOUBLE, i, TAG, MPI_COMM_WORLD);
      offset += stripSize;
    }
  }
  else {  // receive my part of A
    MPI_Recv(A[0], stripSize * N, MPI_DOUBLE, 0, TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
  }
#endif
  
  // everyone gets B
  MPI_Bcast(B[0], N*N, MPI_DOUBLE, 0, MPI_COMM_WORLD);

  // Allocate thread handles
  threads = (pthread_t *) malloc(numThreads * sizeof(pthread_t));
  params = (int *) malloc(numThreads * sizeof(int));

  // Create threads
  for (i = 0; i < numThreads; i++) {
    params[i] = i;
    pthread_create(&threads[i], NULL, worker, (void *)(&params[i]));
  }

  for (i = 0; i < numThreads; i++) {
    pthread_join(threads[i], NULL);
  }

  MPI_Gather(C[0], stripSize * N, MPI_DOUBLE, C[0], stripSize * N, MPI_DOUBLE, 0, MPI_COMM_WORLD);

#if 0
  // administrator receives from workers -- done via MPI_Gather in this example
  // left here (but commented out) to make clear what MPI_Gather is doing
  if (myId == 0) {
    offset = stripSize; 
    numElements = stripSize * N;
    for (i=1; i<numProcesses; i++) {
      MPI_Recv(C[offset], numElements, MPI_DOUBLE, i, TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
      offset += stripSize;
    }
  }
  else { // send worker's piece of C to administrator
    MPI_Send(C[0], stripSize * N, MPI_DOUBLE, 0, TAG, MPI_COMM_WORLD);
  }
#endif

  // stop timer
  if (myId == 0) {
    endTime = MPI_Wtime();
    printf("Time is %f\n", endTime-startTime);
  }
  
  // print out matrix here, if I'm the administrator
  if (myId == 0 && N < 10) {
    printMatrix(C, N, N);
  }
  
  MPI_Finalize();
  return 0;
}


