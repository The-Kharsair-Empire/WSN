
all: run

run: compile
	mpirun -n 21 a.out

compile:
	@#mpicc -c WSN.c functions.c node_functions.c encryption.c -lm
	mpicc -fopenmp  functions.c encryption.c WSN.c  node_functions.c base_station_functions.c -lm

