#!/bin/bash


NUMTHREADS=4
GRIDSIZE=1024
NUMITERS=2048




echo ""
echo "#Threads: ${NUMTHREADS} Grid Size: ${GRIDSIZE} Number of Iterations: ${NUMITERS}"

echo "Running Multi-Threaded"
./mt-jacobi $GRIDSIZE $NUMITERS $NUMTHREADS


echo ""
echo "Running Sequential"
./seq-jacobi $GRIDSIZE $NUMITERS $NUMTHREADS




