# Makefile
# Author: Alan Manuel Loreto Cornídez
# Purpose: This file contains make commands for each of the 
# different executables for generating the jacobian of a matrix.
# Assignment: CSC 522 | Parallel Computing
#
#
#
CC=gcc
CFLAGS= -Wall -O2 -Wfatal-errors

# seq-jacobi: seq-jacobi.c
	# $(CC) $(CFLAGS) -o seq-jacobi seq-jacobi.c

mt-jacobi: mt-jacobi.c
	$(CC) $(CFLAGS) -o mt-jacobi mt-jacobi.c

# dist-jacobi: dist-jacobi.c
	# $(CC) $(CFLAGS) -o dist-jacobi dist-jacobi.c

# hybrid-jacobi: hybrid-jacobi.c
	# $(CC) $(CFLAGS) -o hybrid-jacobi hybrid-jacobi.c


.PHONY: clean	
clean: 
	rm seq-jacobi mt-jacobi dist-jacobi hybrid-jacobi 


.PHONY: all
all: seq-jacobi mt-jacobi dist-jacobi hybrid-jacobi

.DEFAULT_GOAL := all


