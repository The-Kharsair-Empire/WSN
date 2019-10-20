# CC=clang
# MPI=mpicc
# CFLAGS=-Wall -g

#program: WSN.c node_functions.c functions.c encryption.c
#	export OMP_NUM_THREADS = 4
#	sudo mpicc -fopenmp WSN.c node_functions.c functions.c encryption.c
#	mpirun -n 21 ./a.out

all: run

run: compile
	mpirun -n 21 a.out

compile:
	@#mpicc -c WSN.c functions.c node_functions.c encryption.c -lm
	mpicc -fopenmp  functions.c encryption.c WSN.c  node_functions.c base_station_functions.c -lm

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
