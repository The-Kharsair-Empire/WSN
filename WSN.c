

#include "WSN.h"

//WSN grid stucture: 4 x 5;
//An event happens when 3 adjacent nodes report the same integer number to the reference node, only work for integer!;



int main(int argc, char* argv[]){
    int adjacent_node_up, adjacent_node_down, adjacent_node_left, adjacent_node_right;
    int col_pos;
    const int row_len = 5, col_len = 4, num_node = 21, simulation_times = 10;
    const int Base_station = 20; //grid nodes are node 0 to node 19;
    const int window_size = 3;
    const int upperbound = 50; //upperbound of the random number generated
    int rank, size;
    const int Internal_Comm = 0;
    const int Event_Comm = 1;
   
    const int Completion = 3;
    FILE *fp;
    
    MPI_Status stat;
    int rc;
    int *sender_list;
    int *encryption_key;
    int adjacent_num;

    //encryption sections
    long *keys;
    long public_key;
    long private_key;
    long n;
    long *n_mods;
    char IP[20];

    rc = MPI_Init(&argc, &argv);

    if (rc != MPI_SUCCESS){
        printf ("Error starting MPI program. Terminating.\n");
        MPI_Abort(MPI_COMM_WORLD, rc);
    }

    MPI_Comm_size( MPI_COMM_WORLD, &size);
    MPI_Comm_rank( MPI_COMM_WORLD, &rank);

    if (size != num_node){// it has to be 21 nodes
        printf ("Error, please set number of process = %d\n", num_node);
        MPI_Finalize();
    }
    getIP(IP);
    generate_keys(rank, &public_key, &private_key, &n);
    int offset;
    double start_exchange = 0;
    double end_exchange = 0;
    double exchange_time = 0;
    if (rank != Base_station) {//determine each node's position in the grid;
        sender_list = (int*)malloc(4*sizeof(int));
        keys = (long*) malloc(4*sizeof(long));
        n_mods = (long*)  malloc(4*sizeof(long));
        adjacent_num = 0;
        adjacent_node_up = rank - row_len;
        adjacent_node_left = rank - 1;
        adjacent_node_right = rank + 1;
        adjacent_node_down = rank + row_len;
        col_pos = rank % row_len;
        
        offset = 0;
        start_exchange = MPI_Wtime();
        exchange_key(keys, n_mods, &public_key, &n, &adjacent_num, sender_list,  adjacent_node_up, adjacent_node_down, adjacent_node_left, adjacent_node_right, col_pos, Internal_Comm, size, row_len);

        end_exchange = MPI_Wtime();
        exchange_time = end_exchange - start_exchange;
    }

    long node_pub_key = public_key;
    long node_n = n;

    MPI_Bcast(&public_key, 1, MPI_INT, Base_station, MPI_COMM_WORLD);// public key is no longer needed for each sensor node after distributing it to the adjacent node, this variable public key will be used to recieved the public key sent by the base station
    MPI_Bcast(&n, 1, MPI_INT, Base_station, MPI_COMM_WORLD);


    
    
    

    const int msgLen = 412;//message buffer used to pack the message to be encrypted and sent
    char msg1[msgLen];

    char msgbuff[msgLen];
    long code_word1[msgLen];

    char *timestr;
    time_t cur_time;

    if (rank < Base_station) {// nodes functionalities: exchange random number and detect event

        Nodes_functions(window_size, adjacent_num, upperbound, simulation_times, rank, keys, n_mods, private_key, n, adjacent_node_up, adjacent_node_down, adjacent_node_left, adjacent_node_right, col_pos, row_len, size, sender_list,  exchange_time, IP, Internal_Comm, Event_Comm,  Completion, msg1, msgbuff, code_word1, public_key, Base_station);

    } else{// base station received event and log it
        Base_station_functions(simulation_times, upperbound, fp, code_word1, msgLen, private_key, n, Completion, window_size);
    } 


    MPI_Finalize();
    return 0;
}