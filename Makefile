# 
# Sample Makefile for project 1 
#
# The program used for illustration purposes is
# a simple program using prime numbers
#


## CC  = Compiler.
## CFLAGS = Compiler flags.
CC	= gcc
CFLAGS =   -std=c99 -lpthread


## OBJ = Object files.
## SRC = Source files.
## EXE = Executable name.

HRD =       sha256.h uint256.h
SRC =		server.c sha256.c
OBJ =		server.o sha256.o
EXE = 		server

## Top level target is executable.
$(EXE):	$(OBJ)
		$(CC) $(CFLAGS) -o $(EXE) $(OBJ) -lm


## Clean: Remove object files and core dump files.
clean:
		/bin/rm $(OBJ) 

## Clobber: Performs Clean and removes executable file.

clobber: clean
		/bin/rm $(EXE) 

## Dependencies
multi.o: sha256.h uint256.h
sha256.o: sha256.h 
uint256.h: sha256.h