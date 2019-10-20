#include<stdio.h>
#include <stdlib.h> 
#include <math.h>
#include <time.h>
#include <unistd.h>
#include <string.h>
#include <mpi.h>
#include "omp.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <netinet/in.h>
#include <arpa/inet.h>


void generate_keys(int rank, long *public_key, long *private_key, long*n);
void decrypt_cipher(long * input, long private_key, long n, char * output, int len);
void get_cipher(char *input, long public_key, long n, long *output, int len);
long encrypt_one(int input, long public_key, long n);
int decrypt_one(long input, long private_key, long n);

void getIP(char *IP);
void reverse(char *str, int len);
int intToStr(int x, char str[], int significantNum);
void ftoStr(double n, char *res, int significantNum);
char* substring(int start, int end, char a_string[]);

void distribute_key_sendfirst(long * public_key, long * n, long *keys, long *n_mods, int offset, int to_and_from, MPI_Status * stat, int tag);
void distribute_key_recvfirst(long * public_key, long * n, long *keys, long *n_mods, int offset, int to_and_from, MPI_Status * stat, int tag);
void send_random_num_sendfirst(int * random_num, int to_and_from, int tag, long *public_key, long private_key, long *n, int offset, int *recv_buff, MPI_Status * stat);
void encrypt_toAdj(int random_num, long *code_word, long public_key, long n);
void decrypt_fromAdj(long *code_word, int *recv_buff, long private_key, long n);
void exchange_key(long *keys, long *n_mods, long * public_key, long *n, int * adjacent_num, int * sender_list,  int adjacent_node_up, int adjacent_node_down, int adjacent_node_left, int adjacent_node_right, int col_pos, int Internal_Comm, int size, int row_len);
void send_recv_random_num(int *total_msg_to_adj, int *total_msg_from_adj, int random_num, long *keys, long *n_mods, long private_key, long n, int *recv_buff, int adjacent_node_up, int adjacent_node_down, int adjacent_node_left, int adjacent_node_right, int size, int col_pos, int row_len, int Internal_Comm);
void Nodes_functions(int window_size, int adjacent_num, int upperbound, int simulation_times, int rank, long * keys, long *n_mods, long private_key, long n, int adjacent_node_up, int adjacent_node_down, int adjacent_node_left, int adjacent_node_right, int col_pos, int row_len, int size, int *sender_list, double exchange_time, char * IP, int Internal_Comm,int Event_Comm, int Completion, char * msg1, char * msgbuff, long * code_word1, long public_key, int Base_station);


void Base_station_functions(int simulation_times, int upperbound, FILE *fp, long *code_word1, int msgLen, long private_key, long n, int Completion, int window_size);