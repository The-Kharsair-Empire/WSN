#include "WSN.h"

//WSN grid stucture: 4 x 5;
//An event happens when 3 adjacent nodes report the same integer number to the reference node, only work for integer!;



int main(int argc, char* argv[]){
    int adjacent_node_up, adjacent_node_down, adjacent_node_left, adjacent_node_right;
    int col_pos;
    const int row_len = 5, col_len = 4, num_node = 21, simulation_times = 50;
    const int Base_station = 20; //grid nodes are node 0 to node 19;
    const int Summary = 0;
    const int window_size = 3;
    const int upperbound = 45; //not inclusive, generate random number [0, upperbound)
    int rank, size;
    const int Internal_Comm = 0;
    const int Event_Comm = 1;
    const int Summary_Comm = 2;
    const int Completion = 3;
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
    long *n_mods;

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
    generate_keys(rank, &public_key, &private_key, &n);
    int offset;
    if (rank != Base_station) {
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
        if (adjacent_node_up >= 0){ //distributing key and determine adjacent nodes
            //distribute_key_sendfirst(&public_key, &n, keys, n_mods, offset, adjacent_node_up, &stat, Internal_Comm);
            MPI_Send(&public_key, 1, MPI_LONG, adjacent_node_up, Internal_Comm, MPI_COMM_WORLD);
            MPI_Recv(keys+offset, 1, MPI_LONG, adjacent_node_up, Internal_Comm, MPI_COMM_WORLD, &stat);
            MPI_Send(&n, 1, MPI_LONG, adjacent_node_up, Internal_Comm, MPI_COMM_WORLD);
            MPI_Recv(n_mods+offset, 1, MPI_LONG, adjacent_node_up, Internal_Comm, MPI_COMM_WORLD, &stat);
            sender_list[offset++] = adjacent_node_up; 
            adjacent_num++;
        }
        if (adjacent_node_down < size-1){
            MPI_Recv(keys+offset, 1, MPI_LONG, adjacent_node_down, Internal_Comm, MPI_COMM_WORLD, &stat);
            MPI_Send(&public_key, 1, MPI_LONG, adjacent_node_down, Internal_Comm, MPI_COMM_WORLD);
            MPI_Recv(n_mods+offset, 1, MPI_LONG, adjacent_node_down, Internal_Comm, MPI_COMM_WORLD, &stat);
            MPI_Send(&n, 1, MPI_LONG, adjacent_node_down, Internal_Comm, MPI_COMM_WORLD);
            sender_list[offset++] = adjacent_node_down;
            adjacent_num++;
        }
        if (/*adjacent_node_left >= 0 &&*/ col_pos > 0){
            MPI_Send(&public_key, 1, MPI_LONG, adjacent_node_left, Internal_Comm, MPI_COMM_WORLD);
            MPI_Recv(keys+offset, 1, MPI_LONG, adjacent_node_left, Internal_Comm, MPI_COMM_WORLD, &stat);
            MPI_Send(&n, 1, MPI_LONG, adjacent_node_left, Internal_Comm, MPI_COMM_WORLD);
            MPI_Recv(n_mods+offset, 1, MPI_LONG, adjacent_node_left, Internal_Comm, MPI_COMM_WORLD, &stat);
            sender_list[offset++] = adjacent_node_left;
            adjacent_num++;
        }
        if (/*adjacent_node_right < size-1 && */col_pos < row_len-1){
            MPI_Recv(keys+offset, 1, MPI_LONG, adjacent_node_right, Internal_Comm, MPI_COMM_WORLD, &stat);
            MPI_Send(&public_key, 1, MPI_LONG, adjacent_node_right, Internal_Comm, MPI_COMM_WORLD);
            MPI_Recv(n_mods+offset, 1, MPI_LONG, adjacent_node_right, Internal_Comm, MPI_COMM_WORLD, &stat);
            MPI_Send(&n, 1, MPI_LONG, adjacent_node_right, Internal_Comm, MPI_COMM_WORLD);
            sender_list[offset] = adjacent_node_right;
            adjacent_num++;
        }
    }

    MPI_Bcast(&public_key, 1, MPI_INT, Base_station, MPI_COMM_WORLD);// public key is no longer needed for each sensor node after distributing it to the adjacent node, this variable public key will be used to recieved the public key sent by the base station
    MPI_Bcast(&n, 1, MPI_INT, Base_station, MPI_COMM_WORLD);
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
    long code_word;
    long recv;

    int event_num = 0;
    int iteration_event_num = 0;

    int event_node;
    int finished_nodes;
    const int msgLen = 1024;
    char msg1[msgLen];
    char msg2[msgLen];
    char msg3[msgLen];
    char msg4[msgLen];
    char msgbuff[256];
    long code_word1[msgLen];
    long code_word2[msgLen];
    long code_word3[msgLen];
    long code_word4[msgLen];

    //for each node:
    int total_event;
    double total_encryption_time;
    int total_msg_to_adj;
    int total_msg_from_adj;
    int total_msg_to_base;

    char *timestr;
    time_t cur_time;

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
                total_msg_to_adj++; total_msg_from_adj++;
                MPI_Send(&random_num, 1, MPI_INT, adjacent_node_up, Internal_Comm, MPI_COMM_WORLD);
                MPI_Recv(recv_buff+offset, 1, MPI_INT, adjacent_node_up, Internal_Comm, MPI_COMM_WORLD, &stat);
                //send_random_num_sendfirst(&random_num, adjacent_node_up, Internal_Comm, keys, private_key, n_mods, offset, recv_buff, &stat);
                /*code_word = encrypt_one(random_num, keys[offset], n_mods[offset]);*/
              /*  MPI_Send(&code_word, 1, MPI_LONG, adjacent_node_up, Internal_Comm, MPI_COMM_WORLD);
                MPI_Recv(&code_word, 1, MPI_LONG, adjacent_node_up, Internal_Comm, MPI_COMM_WORLD, &stat);*/
              /*  recv_buff[offset] = decrypt_one(code_word, private_key, n);*/
            
                offset++;
            }
            if (adjacent_node_down < size-1){//send and recv random number to and from adjacent_node_down
                total_msg_to_adj++; total_msg_from_adj++;
                MPI_Recv(recv_buff+offset, 1, MPI_INT, adjacent_node_down, Internal_Comm, MPI_COMM_WORLD, &stat);
                MPI_Send(&random_num, 1, MPI_INT, adjacent_node_down, Internal_Comm, MPI_COMM_WORLD);
        
               /* MPI_Recv(&recv, 1, MPI_LONG, adjacent_node_down, Internal_Comm, MPI_COMM_WORLD, &stat);*/
               /* recv_buff[offset] = decrypt_one(recv, private_key, n);
                code_word = encrypt_one(random_num, keys[offset], n_mods[offset]);*/
                /*MPI_Send(&code_word, 1, MPI_LONG, adjacent_node_down, Internal_Comm, MPI_COMM_WORLD);*/
                offset++;
            }
            if (/*adjacent_node_left >= 0 &&*/ col_pos > 0){//send and recv random number to and from adjacent_node_left
                total_msg_to_adj++; total_msg_from_adj++;
                MPI_Send(&random_num, 1, MPI_INT, adjacent_node_left, Internal_Comm, MPI_COMM_WORLD);
                MPI_Recv(recv_buff+offset, 1, MPI_INT, adjacent_node_left, Internal_Comm, MPI_COMM_WORLD, &stat);
                /*code_word = encrypt_one(random_num, keys[offset], n_mods[offset]);*/
                /*MPI_Send(&code_word, 1, MPI_LONG, adjacent_node_left, Internal_Comm, MPI_COMM_WORLD);
                MPI_Recv(&code_word, 1, MPI_LONG, adjacent_node_left, Internal_Comm, MPI_COMM_WORLD, &stat);*/
                /*recv_buff[offset] = decrypt_one(code_word, private_key, n);*/
               
                offset++;
            }

            if (/*adjacent_node_right < size-1 && */col_pos < row_len-1){//send and recv random number to and from adjacent_node_right
                total_msg_to_adj++; total_msg_from_adj++;
                MPI_Recv(recv_buff+offset, 1, MPI_INT, adjacent_node_right, Internal_Comm, MPI_COMM_WORLD, &stat);
                MPI_Send(&random_num, 1, MPI_INT, adjacent_node_right, Internal_Comm, MPI_COMM_WORLD);
               /* MPI_Recv(&recv, 1, MPI_LONG, adjacent_node_right, Internal_Comm, MPI_COMM_WORLD, &stat);*/
                /*recv_buff[offset] = decrypt_one(recv, private_key, n);
                code_word = encrypt_one(random_num, keys[offset], n_mods[offset]);*/
               /* MPI_Send(&code_word, 1, MPI_LONG, adjacent_node_right, Internal_Comm, MPI_COMM_WORLD);*/
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
                        total_event++;
                        total_msg_to_base++;
                        event_time = MPI_Wtime();
                        cur_time = time(NULL);
                        timestr = ctime(&cur_time);
                        timestr[strlen(timestr)-1] = '\0';
    
                        //strcat(timestr, "\0");
                        
                        sprintf(msg1, "[Event Log]\nEvent detected at Node %d at DateTime (%s)", rank, timestr);
                        /*for ( event_sender_pointer=0; event_sender_pointer < adjacent_num; event_sender_pointer++){
                            sprintf(msgbuff, "Node %d, ", sender_list[event_sender_pointer]);
                            strcat(msg1, msgbuff);
                        }
                        */
                        strcat(msg1, "\nAdjacent nodes that triggered the event are: ");
                        for (event_sender_pointer=0; event_sender_pointer < events[rand_num_counter]; event_sender_pointer++){
                            sprintf(msgbuff, "Node %d, ", event_senders[event_sender_pointer]);
                            strcat(msg1, msgbuff);
                        }
                        strcat(msg1, "\nActivation value is :");
                        sprintf(msgbuff, "%d\n", rand_num_counter);
                        strcat(msg1, msgbuff);
                        sprintf(msg2, "This event was detected at exactly %lf computer time second", event_time);

                        get_cipher(msg1, public_key, n, code_word1, strlen(msg1));
                        get_cipher(msg2, public_key, n, code_word2, strlen(msg2));


                       
                        MPI_Send(code_word1, strlen(msg1), MPI_LONG, Base_station, Event_Comm, MPI_COMM_WORLD);
                    }
                }
            }

            sleep(0.01);
            if (rank == Summary){
                total_msg_to_base++;
              
         
                sprintf(msg4,"[Iteration Summary Log]\nthis is end of iteration %d\n", num_of_time);
                //MPI_Send(msg4, strlen(msg), MPI_CHAR, Base_station, Summary_Comm, MPI_COMM_WORLD);
            }
        }
        sleep(0.01);
        total_msg_to_base;
        event_time = MPI_Wtime();
        cur_time = time(NULL);
        timestr = ctime(&cur_time);
        timestr[strlen(timestr)-1] = '\0';
        sprintf(msg3, "[Node Termination Log]");
            /*\nNode %d terminates at DateTime %s, At exactly %lf computer second\nThis Node Detected totally %d events\nTotal number of messages communicate with Adjacent Node: Send %d, Recv %d\nTotal number of message sent to the base station %d\n Total encryption time is: %lf\n", rank, timestr, event_time, total_event, total_msg_to_adj, total_msg_from_adj, total_msg_to_base, total_encryption_time);
        */
        get_cipher(msg3, public_key, n, code_word3, strlen(msg3));
        MPI_Send(code_word3, strlen(msg3), MPI_LONG, Base_station, Completion, MPI_COMM_WORLD);
    } else{
        finished_nodes = 0;
        int wordLen;
        char msg;
        fp = fopen("logfile.txt", "w");
        while (finished_nodes < 20){
            MPI_Recv(code_word1, msgLen, MPI_LONG, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &stat);
            MPI_Get_count(&stat, MPI_LONG, &wordLen);
            char *msg = (char*)malloc(sizeof(char)*wordLen);
            decrypt_cipher(code_word1, private_key, n, msg, wordLen);
        
            //fprintf(fp, "%s", msg);
            printf("%s", msg);
            if (stat.MPI_TAG == Completion){
                finished_nodes++;
                // printf("rank %d has finished\n", event_node);
            } else if (stat.MPI_TAG == Summary_Comm){

                iteration_event_num = 0;
                
                // printf("rank %d has an event\n", event_node);
            } else{
                iteration_event_num++;
                event_num++;
            }
            
        }

        fclose(fp);
    } 
    if (rank == Base_station){
        printf("%d\n", event_num);
    }
    MPI_Finalize();
    return 0;
}