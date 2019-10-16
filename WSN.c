#include "WSN.h"

//WSN grid stucture: 4 x 5;
//An event happens when 3 adjacent nodes report the same integer number to the reference node, only work for integer!;



int main(int argc, char* argv[]){
    int adjacent_node_up, adjacent_node_down, adjacent_node_left, adjacent_node_right;
    int col_pos;
    const int row_len = 5, col_len = 4, num_node = 21, simulation_times = 10;
    const int Base_station = 20; //grid nodes are node 0 to node 19;
    const int window_size = 3;
    const int upperbound = 30; 
    int rank, size;
    const int Internal_Comm = 0;
    const int Event_Comm = 1;
   
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
    char IP[20];

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
    getIP(IP);
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

    const int msgLen = 512;
    char msg1[msgLen];
    char msg2[msgLen];
    char msg3[msgLen];
    char msg4[msgLen];
    char msgbuff[msgLen];
    long code_word1[msgLen];
    long code_word2[msgLen];
    long code_word3[msgLen];
    long code_word4[msgLen];

    //for each node:
    /*int total_event = 0;
    double total_encryption_time;
    int total_msg_to_adj = 0;
    int total_msg_from_adj = 0;
    int total_msg_to_base = 0;
    double start_encryption;
    double end_encryption;
    double encryption_time;

    double total_comm_time_with_adj = 0.0;
    double start_comm;
    double end_comm;*/

    

    //for base station
    

    char *timestr;
    time_t cur_time;

    if (rank < Base_station) {

        int total_event = 0;
        double total_encryption_time;
        int total_msg_to_adj = 0;
        int total_msg_from_adj = 0;
        int total_msg_to_base = 0;
        double start_encryption;
        double end_encryption;
        double encryption_time;

        double total_comm_time_with_adj = 0.0;
        double start_comm;
        double end_comm;
        memset( slider, -1, window_size*adjacent_num*sizeof(int) );
        memset( event_counter, 0, upperbound*adjacent_num*sizeof(int) );

        for (num_of_time = 0; num_of_time < simulation_times; num_of_time++){
        
            seed = rank+num_of_time;
            srand(time(NULL)+seed);
            random_num = rand() % upperbound+1;
            // printf("rank %d, rnd num%d\n", rank, random_num);
        
            recv_buff = (int*)malloc(adjacent_num*sizeof(int));
            offset=0;

            start_comm = MPI_Wtime();

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

            end_comm = MPI_Wtime();
            total_comm_time_with_adj += (end_comm - start_comm);

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
                        
                        cur_time = time(NULL);
                        timestr = ctime(&cur_time);
                        timestr[strlen(timestr)-1] = '\0';
    
                        //strcat(timestr, "\0");
                        
                        sprintf(msg1, "[Event Log at Node %d (DateTime: %s)]\nAt iteration %d\n", rank, timestr, num_of_time);
                        
                        strcat(msg1, "Adjacent nodes that triggered the event are: ");
                        for (event_sender_pointer=0; event_sender_pointer < events[rand_num_counter]; event_sender_pointer++){
                            sprintf(msgbuff, "Node %d, ", event_senders[event_sender_pointer]);
                            strcat(msg1, msgbuff);
                        }
                        strcat(msg1, "\nActivation value is: ");
                        sprintf(msgbuff, "%d\n", rand_num_counter);
                        strcat(msg1, msgbuff);
                        
                        start_encryption = MPI_Wtime();

                        get_cipher(msg1, public_key, n, code_word1, strlen(msg1));
                        

                        end_encryption = MPI_Wtime();
                        /*printf("end_en %lf\n", end_encryption);
                        printf("start_en %lf\n", start_encryption);*/
                        event_time = MPI_Wtime();
                        encryption_time = end_encryption - start_encryption;
                        total_encryption_time += encryption_time;
                        MPI_Send(code_word1, strlen(msg1), MPI_LONG, Base_station, Event_Comm, MPI_COMM_WORLD);

                        sprintf(msg2, "%d$%lf$%lf$", num_of_time, encryption_time, event_time);//iteration, encryption time, send time
                        get_cipher(msg2, public_key, n, code_word2, strlen(msg2));
                        
                        MPI_Send(code_word2, strlen(msg2), MPI_LONG, Base_station, Event_Comm, MPI_COMM_WORLD);

                    }

                }
                /*free(events);
                free(event_senders);*/
                
            }
            
           /* free(recv_buff);*/

            sleep(0.01);
            
        }
        total_msg_to_base++;
        event_time = MPI_Wtime();
        cur_time = time(NULL);
        timestr = ctime(&cur_time);
        timestr[strlen(timestr)-1] = '\0';
        sprintf(msg3, "[Node Termination Log at Node %d (DateTime: %s)]\nNode Summary:\nNode IP:%s\n%d Events detected\n%d Messages sent.\n%d Messages received\nTotal communication time With Adjacent Nodes: %lf\nAdjacent Node:",rank,  timestr, IP, total_event, (total_msg_to_base + total_msg_to_adj), total_msg_from_adj, total_comm_time_with_adj);
        for ( event_sender_pointer=0; event_sender_pointer < adjacent_num; event_sender_pointer++){
            sprintf(msgbuff, " Node %d,", sender_list[event_sender_pointer]);
            strcat(msg3, msgbuff);
        }
                        
        strcat(msg3, "\n\n");

        get_cipher(msg3, public_key, n, code_word3, strlen(msg3));
        MPI_Send(code_word3, strlen(msg3), MPI_LONG, Base_station, Completion, MPI_COMM_WORLD);
    } else{
        double total_decryption_time = 0.0;
        double decryption_time;
        double start_decryption;

        double end_decryption;
/*
        char *summary_header = ;

        double summary_table[simulation_times][8] = memset();*/


        double recv_time;
        int break_point;
        int finished_nodes = 0;
        int wordLen;
        char msg;
        int item_i;

        fp = fopen("logfile.txt", "w");
        while (finished_nodes < 20){
            wordLen = 0;
            MPI_Recv(code_word1, msgLen, MPI_LONG, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &stat);
            MPI_Get_count(&stat, MPI_LONG, &wordLen);
            char *msg = (char*)malloc(sizeof(char)*msgLen); //wordLen
            printf("word len %d\n", wordLen);
            start_decryption = MPI_Wtime();
            decrypt_cipher(code_word1, private_key, n, msg, wordLen);
            end_decryption = MPI_Wtime();
            decryption_time = end_decryption - start_decryption;
            total_decryption_time += decryption_time;
        
            fprintf(fp, "%s", msg);
            printf("%s", msg);
            if (stat.MPI_TAG == Completion){
                finished_nodes++;
                // printf("rank %d has finished\n", event_node);
            }  else{
                MPI_Recv(code_word2, msgLen, MPI_LONG, stat.MPI_SOURCE, stat.MPI_TAG, MPI_COMM_WORLD, &stat);
                recv_time = MPI_Wtime();
                MPI_Get_count(&stat, MPI_LONG, &wordLen);
                msg = (char*)malloc(sizeof(char)*wordLen);
                decrypt_cipher(code_word2, private_key, n, msg, wordLen);
                break_point = 0;
                item_i = 0;
                for (int i =0;i < wordLen; i++){
                    if( msg[i] == '$'){
                        char *sec = substring(break_point, i, msg);

                        
                        if (item_i == 0){
                            printf("%s\n", sec);

                            item_i++;
                        }else if (item_i == 1){
                            printf("%s\n", sec);
                            fprintf(fp, "Encryption time: %lf\nDecryption time %lf\n", atof(sec), decryption_time);
                            item_i ++;
                        }else {
                            printf("%s\n", sec);
                            fprintf(fp, "Time Taken to report Event (communication time From Event Reporting Node to Base Station): %lf\n\n", recv_time- (atof(sec)));

                        }

                        break_point = i+1;

                    }
                }
                free(msg);
            
                event_num++;
            }
            
        }

        fclose(fp);
        printf("%d\n", event_num);
    } 


    MPI_Finalize();
    return 0;
}