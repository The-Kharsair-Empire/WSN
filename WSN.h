#include<stdio.h>
#include <stdlib.h> 
#include <math.h>
#include <time.h>
#include <unistd.h>
#include <string.h>
#include "omp.h"

void generate_keys(int rank, long *public_key, long *private_key, long*n);
void decrypt_cipher(long * input, long private_key, long n, char * output, int len);
void get_cipher(char *input, long public_key, long n, long *output, int len);

void reverse(char *str, int len);
int intToStr(int x, char str[], int significantNum);
void ftoStr(double n, char *res, int significantNum);