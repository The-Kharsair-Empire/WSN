# CC=clang
# MPI=mpicc
# CFLAGS=-Wall -g

all: run

run: compile
	mpirun -n 21 a.out

compile:

	mpicc -c WSN.c functions.c encryption.c
	mpicc -openmp WSN.o functions.o encryption.o

# mainapp: WSN.o functions.o
# 	$(MPI) WSN.o functions.o encryption.o

# WSN.o: WSN.c
# 	$(MPI) -c WSN.c

# functions.o: functions.c
# 	$(CC) -c functions.c

# encryption.o: encryption.c
# 	$(CC) -fopenmp -c encryption.c

# clean:
# 	rm a.out *.o mainapp
