#include<stdio.h>
// #include <omp.h> 
#include <stdlib.h> 
#include <math.h>
#include <time.h>
#include <unistd.h>
#include <string.h>
#include <mpi.h>

//WSN grid stucture: 4 x 5;
//An event happens when 3 adjacent nodes report the same integer number to the reference node, only work for integer!;



int main(int argc, char* argv[]){
    int adjacent_node_up, adjacent_node_down, adjacent_node_left, adjacent_node_right;
    int col_pos;
    const int row_len = 5, col_len = 4, num_node = 21, simulation_times = 1000;
    const int Base_station = 20; //grid nodes are node 0 to node 19;
    const int window_size = 3;
    const int upperbound = 45; //not inclusive, generate random number [0, upperbound)
    int rank, size;
    const int Internal_Comm = 0;
    const int ToBaseStation_Comm = 1;
    const int Completion = 2;
    FILE *fp;
    
    MPI_Status stat;
    int rc;
    int *recv_buff;
    int *sender_list;
    int *encryption_key;
    int adjacent_num;

    //encryption sections
    long *keys;
    long public_key;
    long private_key;
    long n;

    rc = MPI_Init(&argc, &argv);

    if (rc != MPI_SUCCESS){
        printf ("Error starting MPI program. Terminating.\n");
		MPI_Abort(MPI_COMM_WORLD, rc);
    }

    MPI_Comm_size( MPI_COMM_WORLD, &size);
	MPI_Comm_rank( MPI_COMM_WORLD, &rank);

    if (size != num_node){
        printf ("Error, please set number of process = %d\n", num_node);
		MPI_Finalize();
    }
    // generate_keys(rank, &public_key, &private_key, &n);
    int offset;
    if (rank != Base_station) {
        sender_list = (int*)malloc(4*sizeof(int));
        keys = (long*) malloc(4*sizeof(long));
        adjacent_num = 0;
        adjacent_node_up = rank - row_len;
        adjacent_node_left = rank - 1;
        adjacent_node_right = rank + 1;
        adjacent_node_down = rank + row_len;
        col_pos = rank % row_len;
        
        offset = 0;
        if (adjacent_node_up >= 0){ //distributing key and determine adjacent nodes
            sender_list[offset++] = adjacent_node_up; 
            adjacent_num++;
            
        }
        if (adjacent_node_down < size-1){
            sender_list[offset++] = adjacent_node_down;
            adjacent_num++;
        }
        if (/*adjacent_node_left >= 0 &&*/ col_pos > 0){
            sender_list[offset++] = adjacent_node_left;
            adjacent_num++;
        }
        if (/*adjacent_node_right < size-1 && */col_pos < row_len-1){
            sender_list[offset] = adjacent_node_right;
            adjacent_num++;
        }
    }
    //all node sends ip to the base station;
    
    int num_of_time;
    int random_num;
    int seed;
    int event_iterator, rand_num_counter;
    int slider[window_size][adjacent_num];
    int event_counter[upperbound][adjacent_num];
    int* events;
    int* event_senders;
    int event_sender_pointer;
    int slider_pointer;
    double event_time;

    int event_num = 0;

    int event_node;
    int finished_nodes;

    if (rank != Base_station) {
        memset( slider, -1, window_size*adjacent_num*sizeof(int) );
        memset( event_counter, 0, upperbound*adjacent_num*sizeof(int) );

        for (num_of_time = 0; num_of_time < simulation_times; num_of_time++){
        
            seed = rank+num_of_time;
            srand(time(NULL)+seed);
            random_num = rand() % upperbound;
            // printf("rank %d, rnd num%d\n", rank, random_num);
        
            recv_buff = (int*)malloc(adjacent_num*sizeof(int));
            offset=0;

            if (adjacent_node_up >= 0){ //send and recv random number to and from adjacent_node_up
                MPI_Send(&random_num, 1, MPI_INT, adjacent_node_up, Internal_Comm, MPI_COMM_WORLD);
                MPI_Recv(recv_buff+offset, 1, MPI_INT, adjacent_node_up, Internal_Comm, MPI_COMM_WORLD, &stat);
                offset++;

            }
            if (adjacent_node_down < size-1){//send and recv random number to and from adjacent_node_down
                MPI_Recv(recv_buff+offset, 1, MPI_INT, adjacent_node_down, Internal_Comm, MPI_COMM_WORLD, &stat);
                MPI_Send(&random_num, 1, MPI_INT, adjacent_node_down, Internal_Comm, MPI_COMM_WORLD);
                offset++;

            }
            if (/*adjacent_node_left >= 0 &&*/ col_pos > 0){//send and recv random number to and from adjacent_node_left
                MPI_Send(&random_num, 1, MPI_INT, adjacent_node_left, Internal_Comm, MPI_COMM_WORLD);
                MPI_Recv(recv_buff+offset, 1, MPI_INT, adjacent_node_left, Internal_Comm, MPI_COMM_WORLD, &stat);
                offset++;
            }

            if (/*adjacent_node_right < size-1 && */col_pos < row_len-1){//send and recv random number to and from adjacent_node_right
                MPI_Recv(recv_buff+offset, 1, MPI_INT, adjacent_node_right, Internal_Comm, MPI_COMM_WORLD, &stat);
                MPI_Send(&random_num, 1, MPI_INT, adjacent_node_right, Internal_Comm, MPI_COMM_WORLD);
            }

            if (adjacent_num > 2) { 

                events = (int*)calloc(upperbound, sizeof(int));
                
                event_senders = (int*)malloc(adjacent_num*sizeof(int));

                for(event_iterator = 0; event_iterator < adjacent_num; event_iterator++){
                    if (slider[num_of_time % window_size][event_iterator] > -1){
                        event_counter[slider[num_of_time % window_size][event_iterator]][event_iterator]--;
                    }
                    slider[num_of_time % window_size][event_iterator] = recv_buff[event_iterator];
                    event_counter[slider[num_of_time % window_size][event_iterator]][event_iterator]++;
                }

                for (rand_num_counter = 0; rand_num_counter < upperbound; rand_num_counter++){                
                    for(event_iterator = 0; event_iterator < adjacent_num; event_iterator++){
                        if (event_counter[rand_num_counter][event_iterator] > 0){
                            event_senders[events[rand_num_counter]] = sender_list[event_iterator];
                            events[rand_num_counter]++;
                        }
                    }

                    if (events[rand_num_counter] >= 3){
                        MPI_Send(&rank, 1, MPI_INT, Base_station, ToBaseStation_Comm, MPI_COMM_WORLD);
                    }
                }
            }
        }
        MPI_Send(&rank, 1, MPI_INT, Base_station, Completion, MPI_COMM_WORLD);
    } else{
        finished_nodes = 0;
        while (finished_nodes < 20){
            MPI_Recv(&event_node, 1, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &stat);
            if (stat.MPI_TAG == Completion){
                finished_nodes++;
                // printf("rank %d has finished\n", event_node);
            } else{
                event_num++;
                // printf("rank %d has an event\n", event_node);
            }
        }
    } 
    if (rank == Base_station){
        printf("%d\n", event_num);
    }
    MPI_Finalize();
	return 0;
}