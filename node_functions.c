
#include "WSN.h"
//this files contains the functionalities of nodes/sensors in the WSN grid
void distribute_key_sendfirst(long * public_key, long * n, long *keys, long *n_mods, int offset, int to_and_from, MPI_Status * stat, int tag){
	MPI_Send(public_key, 1, MPI_LONG, to_and_from, tag, MPI_COMM_WORLD);
    MPI_Recv(keys+offset, 1, MPI_LONG, to_and_from, tag, MPI_COMM_WORLD, stat);
    MPI_Send(n, 1, MPI_LONG, to_and_from, tag, MPI_COMM_WORLD);
    MPI_Recv(n_mods+offset, 1, MPI_LONG, to_and_from, tag, MPI_COMM_WORLD, stat);

}

void distribute_key_recvfirst(long * public_key, long * n, long *keys, long *n_mods, int offset, int to_and_from, MPI_Status * stat, int tag){
    MPI_Recv(keys+offset, 1, MPI_LONG, to_and_from, tag, MPI_COMM_WORLD, stat);
    MPI_Send(public_key, 1, MPI_LONG, to_and_from, tag, MPI_COMM_WORLD);
    MPI_Recv(n_mods+offset, 1, MPI_LONG, to_and_from, tag, MPI_COMM_WORLD, stat);
    MPI_Send(n, 1, MPI_LONG, to_and_from, tag, MPI_COMM_WORLD);
}

void send_random_num_sendfirst(int * random_num, int to_and_from, int tag, long *public_key, long private_key, long *n, int offset, int *recv_buff, MPI_Status * stat){
	printf("before %d\n",*random_num);
	char rm = (char) *random_num;
	long recv_val[1] = {0};
	long output[1] = {0};
	char msg[1] = {0};
	get_cipher(msg, public_key[offset], n[offset], output, 1);
	MPI_Send(output, 1, MPI_LONG, to_and_from, tag, MPI_COMM_WORLD);
    MPI_Recv(recv_val, 1, MPI_LONG, to_and_from, tag, MPI_COMM_WORLD, stat);
    decrypt_cipher(recv_val, private_key, *n, msg, 1);
    *(recv_buff+offset) = (int) msg[0];
    printf("after %d\n",*(recv_buff+offset));
}

void encrypt_toAdj(int random_num, long *code_word, long public_key, long n){
	printf("before encrypt %d\n",random_num);
	char rm = (char) random_num;
	char msg[1] = {rm};
	get_cipher(msg, public_key, n, code_word, 1);
	printf("after encrypt %ld\n",code_word[0]);
}

void decrypt_fromAdj(long *code_word, int *recv_buff, long private_key, long n){
	printf("before decrypt %ld\n",*code_word);
	char output[1] = {0};
	decrypt_cipher(code_word, private_key, n, output, 1);
	*recv_buff = (int) output[0];

	printf("after decrypt %d\n",*recv_buff);

}

void exchange_key(long *keys, long *n_mods, long * public_key, long *n, int * adjacent_num, int * sender_list,  int adjacent_node_up, int adjacent_node_down, int adjacent_node_left, int adjacent_node_right, int col_pos, int Internal_Comm, int size, int row_len){
	MPI_Status stat;
	int offset = 0;
	if (adjacent_node_up >= 0){ //distributing key and determine adjacent nodes
        MPI_Send(public_key, 1, MPI_LONG, adjacent_node_up, Internal_Comm, MPI_COMM_WORLD);
        MPI_Recv(keys+offset, 1, MPI_LONG, adjacent_node_up, Internal_Comm, MPI_COMM_WORLD, &stat);
        MPI_Send(n, 1, MPI_LONG, adjacent_node_up, Internal_Comm, MPI_COMM_WORLD);
        MPI_Recv(n_mods+offset, 1, MPI_LONG, adjacent_node_up, Internal_Comm, MPI_COMM_WORLD, &stat);
        sender_list[offset++] = adjacent_node_up; 
        (*adjacent_num)++;
    }
    if (adjacent_node_down < size-1){
        MPI_Recv(keys+offset, 1, MPI_LONG, adjacent_node_down, Internal_Comm, MPI_COMM_WORLD, &stat);
        MPI_Send(public_key, 1, MPI_LONG, adjacent_node_down, Internal_Comm, MPI_COMM_WORLD);
        MPI_Recv(n_mods+offset, 1, MPI_LONG, adjacent_node_down, Internal_Comm, MPI_COMM_WORLD, &stat);
        MPI_Send(n, 1, MPI_LONG, adjacent_node_down, Internal_Comm, MPI_COMM_WORLD);
        sender_list[offset++] = adjacent_node_down;
        (*adjacent_num)++;
    }
    if (/*adjacent_node_left >= 0 &&*/ col_pos > 0){
        MPI_Send(public_key, 1, MPI_LONG, adjacent_node_left, Internal_Comm, MPI_COMM_WORLD);
        MPI_Recv(keys+offset, 1, MPI_LONG, adjacent_node_left, Internal_Comm, MPI_COMM_WORLD, &stat);
        MPI_Send(n, 1, MPI_LONG, adjacent_node_left, Internal_Comm, MPI_COMM_WORLD);
        MPI_Recv(n_mods+offset, 1, MPI_LONG, adjacent_node_left, Internal_Comm, MPI_COMM_WORLD, &stat);
        sender_list[offset++] = adjacent_node_left;
        (*adjacent_num)++;
    }
    if (/*adjacent_node_right < size-1 && */col_pos < row_len-1){
        MPI_Recv(keys+offset, 1, MPI_LONG, adjacent_node_right, Internal_Comm, MPI_COMM_WORLD, &stat);
        MPI_Send(public_key, 1, MPI_LONG, adjacent_node_right, Internal_Comm, MPI_COMM_WORLD);
        MPI_Recv(n_mods+offset, 1, MPI_LONG, adjacent_node_right, Internal_Comm, MPI_COMM_WORLD, &stat);
        MPI_Send(n, 1, MPI_LONG, adjacent_node_right, Internal_Comm, MPI_COMM_WORLD);
        sender_list[offset] = adjacent_node_right;
        (*adjacent_num)++;
    }
}

void send_recv_random_num(int *total_msg_to_adj, int *total_msg_from_adj, int random_num, long *keys, long *n_mods, long private_key, long n, int *recv_buff, int adjacent_node_up, int adjacent_node_down, int adjacent_node_left, int adjacent_node_right, int size, int col_pos, int row_len, int Internal_Comm){
	long code_send;
	int code_recv;
	int offset = 0;
	MPI_Status stat;

    if (adjacent_node_up >= 0){ //send and recv random number to and from adjacent_node_up
        (*total_msg_to_adj)++; (*total_msg_from_adj)++;
        code_send = encrypt_one(random_num, keys[offset], n_mods[offset]);
        MPI_Send(&random_num, 1, MPI_INT, adjacent_node_up, Internal_Comm, MPI_COMM_WORLD);
        MPI_Recv(recv_buff+offset, 1, MPI_INT, adjacent_node_up, Internal_Comm, MPI_COMM_WORLD, &stat);
        code_recv = decrypt_one(code_send, private_key, n);

    
        offset++;
    }
    if (adjacent_node_down < size-1){//send and recv random number to and from adjacent_node_down
 
        (*total_msg_to_adj)++; (*total_msg_from_adj)++;
        MPI_Recv(recv_buff+offset, 1, MPI_INT, adjacent_node_down, Internal_Comm, MPI_COMM_WORLD, &stat);
        code_recv = decrypt_one(code_send, private_key, n);
        MPI_Send(&random_num, 1, MPI_INT, adjacent_node_down, Internal_Comm, MPI_COMM_WORLD);
        code_send = encrypt_one(random_num, keys[offset], n_mods[offset]);

        offset++;
    }
    if (/*adjacent_node_left >= 0 &&*/ col_pos > 0){//send and recv random number to and from adjacent_node_left
        (*total_msg_to_adj)++; (*total_msg_from_adj)++;
        code_send = encrypt_one(random_num, keys[offset], n_mods[offset]);
        MPI_Send(&random_num, 1, MPI_INT, adjacent_node_left, Internal_Comm, MPI_COMM_WORLD);
        MPI_Recv(recv_buff+offset, 1, MPI_INT, adjacent_node_left, Internal_Comm, MPI_COMM_WORLD, &stat);
        code_recv = decrypt_one(code_send, private_key, n);

       
        offset++;
    }

    if (/*adjacent_node_right < size-1 && */col_pos < row_len-1){//send and recv random number to and from adjacent_node_right
        (*total_msg_to_adj)++; (*total_msg_from_adj)++;
        code_recv = decrypt_one(code_send, private_key, n);
        MPI_Recv(recv_buff+offset, 1, MPI_INT, adjacent_node_right, Internal_Comm, MPI_COMM_WORLD, &stat);
        MPI_Send(&random_num, 1, MPI_INT, adjacent_node_right, Internal_Comm, MPI_COMM_WORLD);
        code_send = encrypt_one(random_num, keys[offset], n_mods[offset]);

    }
}

void Nodes_functions(int window_size, int adjacent_num, int upperbound, int simulation_times, int rank, long * keys, long *n_mods, long private_key, long n, int adjacent_node_up, int adjacent_node_down, int adjacent_node_left, int adjacent_node_right, int col_pos, int row_len, int size, int *sender_list, double exchange_time, char * IP, int Internal_Comm,int Event_Comm, int Completion, char * msg1, char * msgbuff, long * code_word1, long public_key, int Base_station){
	int *recv_buff;
	int offset;
	time_t cur_time;
	char *timestr;
    double total_encryption_time = 0.0;
    int total_msg_to_adj = 0;
    int total_msg_from_adj = 0;
    int total_msg_to_base = 0;
    double start_encryption;
    double end_encryption;
    double encryption_time;

    double total_comm_time_with_adj = 0.0;
    double start_comm; 
    double end_comm;
    double event_time;
    int slider[window_size][adjacent_num];
	int event_counter[upperbound][adjacent_num];
	int* events;
	int* event_senders;
	int event_sender_pointer;
	int slider_pointer;

	int num_of_time;
    int random_num;
	int seed;
	int total_event = 0;
	int event_iterator, rand_num_counter;

    memset( slider, -1, window_size*adjacent_num*sizeof(int) );
    memset( event_counter, 0, upperbound*adjacent_num*sizeof(int) );

    long code_send = 0;
    long code_recv = 0;

    for (num_of_time = 0; num_of_time < simulation_times; num_of_time++){
    
        seed = rank+num_of_time;
        srand(time(NULL)+seed);
        random_num = rand() % upperbound+1;
        // printf("rank %d, rnd num%d\n", rank, random_num);
    
        recv_buff = (int*)malloc(adjacent_num*sizeof(int));
        offset=0;

        start_comm = MPI_Wtime();

        send_recv_random_num(&total_msg_to_adj, &total_msg_from_adj, random_num, keys, n_mods, private_key, n, recv_buff, adjacent_node_up, adjacent_node_down, adjacent_node_left, adjacent_node_right, size, col_pos, row_len, Internal_Comm);


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

                    
                    sprintf(msg1, "[Event Summary Log at Node %d (DateTime: %s)]\nAt iteration %d\n", rank, timestr, num_of_time); //string used to pack the information
                    
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

                    event_time = MPI_Wtime();
                    encryption_time = end_encryption - start_encryption;
                    total_encryption_time += encryption_time;
                    MPI_Send(code_word1, strlen(msg1), MPI_LONG, Base_station, Event_Comm, MPI_COMM_WORLD);

                    sprintf(msg1, "%d$%lf$%lf$", num_of_time, encryption_time, event_time);//iteration, encryption time, send time
                    get_cipher(msg1, public_key, n, code_word1, strlen(msg1));
                    
                    MPI_Send(code_word1, strlen(msg1), MPI_LONG, Base_station, Event_Comm, MPI_COMM_WORLD);

                }

            }
            free(events);
            free(event_senders);
            
        }
        
        free(recv_buff);
        sleep(0.01);
        
    }
    total_msg_to_base++;
    event_time = MPI_Wtime();
    cur_time = time(NULL);
    timestr = ctime(&cur_time);
    timestr[strlen(timestr)-1] = '\0';
    sprintf(msg1, "[Node Termination Summary Log at Node %d (DateTime: %s)]\nNode IP:%s\n%d Events detected\n%d Messages sent.\n%d Messages received\nTotal communication time With Adjacent Nodes: %lf\nTotal Key exchange time with Adjacent Nodes: %lf\nAdjacent Node:",rank,  timestr, IP, total_event, (total_msg_to_base + total_msg_to_adj), total_msg_from_adj, total_comm_time_with_adj, exchange_time);
    for ( event_sender_pointer=0; event_sender_pointer < adjacent_num; event_sender_pointer++){
        sprintf(msgbuff, " Node %d,", sender_list[event_sender_pointer]);
        strcat(msg1, msgbuff);
    }
                    
    strcat(msg1, "\n\n");

    get_cipher(msg1, public_key, n, code_word1, strlen(msg1));
    MPI_Send(code_word1, strlen(msg1), MPI_LONG, Base_station, Completion, MPI_COMM_WORLD);

    sprintf(msg1, "%lf", total_encryption_time);
    get_cipher(msg1, public_key, n, code_word1, strlen(msg1));
    MPI_Send(code_word1, strlen(msg1), MPI_LONG, Base_station, Completion, MPI_COMM_WORLD);
}

/*        if (adjacent_node_up >= 0){ //distributing key and determine adjacent nodes
           
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
        if (col_pos > 0){
            MPI_Send(&public_key, 1, MPI_LONG, adjacent_node_left, Internal_Comm, MPI_COMM_WORLD);
            MPI_Recv(keys+offset, 1, MPI_LONG, adjacent_node_left, Internal_Comm, MPI_COMM_WORLD, &stat);
            MPI_Send(&n, 1, MPI_LONG, adjacent_node_left, Internal_Comm, MPI_COMM_WORLD);
            MPI_Recv(n_mods+offset, 1, MPI_LONG, adjacent_node_left, Internal_Comm, MPI_COMM_WORLD, &stat);
            sender_list[offset++] = adjacent_node_left;
            adjacent_num++;
        }
        if (col_pos < row_len-1){
            MPI_Recv(keys+offset, 1, MPI_LONG, adjacent_node_right, Internal_Comm, MPI_COMM_WORLD, &stat);
            MPI_Send(&public_key, 1, MPI_LONG, adjacent_node_right, Internal_Comm, MPI_COMM_WORLD);
            MPI_Recv(n_mods+offset, 1, MPI_LONG, adjacent_node_right, Internal_Comm, MPI_COMM_WORLD, &stat);
            MPI_Send(&n, 1, MPI_LONG, adjacent_node_right, Internal_Comm, MPI_COMM_WORLD);
            sender_list[offset] = adjacent_node_right;
            adjacent_num++;
        }*/


/* if (adjacent_node_up >= 0){ //send and recv random number to and from adjacent_node_up
                total_msg_to_adj++; total_msg_from_adj++;

                code_send = encrypt_one(random_num, keys[offset], n_mods[offset]);
                MPI_Send(&random_num, 1, MPI_INT, adjacent_node_up, Internal_Comm, MPI_COMM_WORLD);
                MPI_Recv(recv_buff+offset, 1, MPI_INT, adjacent_node_up, Internal_Comm, MPI_COMM_WORLD, &stat);
                code_recv = decrypt_one(code_send, private_key, n);

            
                offset++;
            }
            if (adjacent_node_down < size-1){//send and recv random number to and from adjacent_node_down
                total_msg_to_adj++; total_msg_from_adj++;
                MPI_Recv(recv_buff+offset, 1, MPI_INT, adjacent_node_down, Internal_Comm, MPI_COMM_WORLD, &stat);
                code_recv = decrypt_one(code_send, private_key, n);
                MPI_Send(&random_num, 1, MPI_INT, adjacent_node_down, Internal_Comm, MPI_COMM_WORLD);
                code_send = encrypt_one(random_num, keys[offset], n_mods[offset]);
 
                offset++;
            }
            if (col_pos > 0){//send and recv random number to and from adjacent_node_left
                total_msg_to_adj++; total_msg_from_adj++;
                code_send = encrypt_one(random_num, keys[offset], n_mods[offset]);
                MPI_Send(&random_num, 1, MPI_INT, adjacent_node_left, Internal_Comm, MPI_COMM_WORLD);
                MPI_Recv(recv_buff+offset, 1, MPI_INT, adjacent_node_left, Internal_Comm, MPI_COMM_WORLD, &stat);
                code_recv = decrypt_one(code_send, private_key, n);

               
                offset++;
            }

            if (col_pos < row_len-1){//send and recv random number to and from adjacent_node_right
                total_msg_to_adj++; total_msg_from_adj++;
                code_recv = decrypt_one(code_send, private_key, n);
                MPI_Recv(recv_buff+offset, 1, MPI_INT, adjacent_node_right, Internal_Comm, MPI_COMM_WORLD, &stat);
                MPI_Send(&random_num, 1, MPI_INT, adjacent_node_right, Internal_Comm, MPI_COMM_WORLD);
                code_send = encrypt_one(random_num, keys[offset], n_mods[offset]);

            }*/