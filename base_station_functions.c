#include "WSN.h"
//this files contains the all the functionalities of a base station.
void Base_station_functions(int simulation_times, int upperbound, FILE *fp, long *code_word1, int msgLen, long private_key, long n, int Completion, int window_size){
	MPI_Status stat;
	double total_decryption_time = 0.0;

    double decryption_time;
    double start_decryption;

    double end_decryption;

    double total_encryption_time_all_nodes = 0;

    double summary_table[simulation_times][5];

    memset( summary_table, 0.0, simulation_times*5*sizeof(double) );


    double recv_time;
    int break_point;
    int finished_nodes = 0;
    int wordLen;
    char msg;
    int item_i;
    int event_num = 0;

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
            MPI_Recv(code_word1, msgLen, MPI_LONG, stat.MPI_SOURCE, stat.MPI_TAG, MPI_COMM_WORLD, &stat);
            MPI_Get_count(&stat, MPI_LONG, &wordLen);
            msg = (char*)malloc(sizeof(char)*msgLen);
            decrypt_cipher(code_word1, private_key, n, msg, wordLen);
            total_encryption_time_all_nodes += atof(msg);

            // printf("rank %d has finished\n", event_node);
        } else {
            MPI_Recv(code_word1, msgLen, MPI_LONG, stat.MPI_SOURCE, stat.MPI_TAG, MPI_COMM_WORLD, &stat);
            recv_time = MPI_Wtime();
            MPI_Get_count(&stat, MPI_LONG, &wordLen);
            msg = (char*)malloc(sizeof(char)*msgLen);
            decrypt_cipher(code_word1, private_key, n, msg, wordLen);
            break_point = 0;
            item_i = 0;
            for (int i =0;i < wordLen; i++){
                if( msg[i] == '$'){
                    char *sec = substring(break_point, i, msg);
                    int iter;

                    
                    if (item_i == 0){
                        printf("%s\n", sec);
                        iter = (int)  atof(sec);
                        summary_table[iter][1] += 1; 
                        item_i++;
                    }else if (item_i == 1){
                        printf("%s\n", sec);
                        fprintf(fp, "Encryption time: %lf\nDecryption time %lf\n", atof(sec), decryption_time);
                        summary_table[iter][2] += atof(sec); summary_table[iter][3] += decryption_time; 
                        item_i ++;
                    }else {
                        printf("%s\n", sec);
                        fprintf(fp, "Time Taken to report Event (communication time From Event Reporting Node to Base Station): %lf\n\n", recv_time- (atof(sec)));
                        summary_table[iter][4] += (recv_time- (atof(sec))); 

                    }

                    break_point = i+1;

                }
            }
            free(msg);
            event_num++;
        }
        
    }
    //Writing Program Summary Message
    fprintf(fp, "[Program Summary Log]\nSimulation Iteration: %d\nDetection Window Size: %d\nRandom Number Upperbound: %d\nTotal Event Detected During Simulation %d\nTotal Encryption Time: %lf\nTotal Decryption Time: %lf\n\n[Iteration Summary Log]\n", simulation_times, window_size, upperbound, event_num, total_encryption_time_all_nodes, total_decryption_time);
    fprintf(fp, "Iteration\tEvents detected\t\tEncyprtion Time\t\tDecryption Time\t\tCommunication Time\t\tTotal Messages communicated\n");
    int i;
    for (i = 0; i < simulation_times; i++){
        fprintf(fp, "%d\t\t\t%d\t\t\t\t\t%lf\t\t\t%lf\t\t\t%lf", i, (int)summary_table[i][1], summary_table[i][2], summary_table[i][3], summary_table[i][4]);
        fprintf(fp, "\t\t\t%d\n", (int)summary_table[i][1]+62);
    }
    fclose(fp);
    printf("%d\n", event_num);
}