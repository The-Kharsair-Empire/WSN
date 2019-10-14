#include "WSN.h"

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

void get_event_log_msg(long * code_word,  int * len){
	
}

void encrypt_toBaseStation(){

}
