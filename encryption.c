#include "WSN.h"

//this is an encryption algorithm called RSA using asymmetric key, a node will generate a pair of keys: public key and private key
//then it will distribute the public key to its adjacent nodes, when it's receiving msg from its adjacent nodes.
//adjacent nodes will encrypt the msg using the public key, then after this node receive the msg, it decrypts it using the private key
//base station will distribute the key to every other node at the start.
//find gcd
int gcd(long a, long h)
{
    long temp;
    while(1) {
        temp = a%h;
        if(temp==0){
            return h;
        }
        a = h;
        h = temp;
    }
}

// calculate public key: condition, public key < phi_n, pub_key co-prime phi_n, pub_key is odd
//p, q can be seened as seeds, determined by rank
void get_public_key(long p, long q, long *public_key) {
	
	long phi_n = (p-1)*(q-1);
	*public_key = (phi_n)/2;
	long temp = 0;
	while (*public_key != temp) {
		temp = *public_key;
		*public_key = ((phi_n)/temp+temp)/2;
	}
	// *public_key = (int)public_key;
	while (gcd(*public_key, phi_n) != 1){
		(*public_key)++;
	}
}

// calculate private key based on generated public key
//simply: private_key = (1 + (2*phi_n))/public_key, but have to go through key expansion;
void get_private_key(long p, long q, long public_key, long *private_key) {

	long phi_n = (p-1)*(q-1);
	*private_key = 1;
	long tmp = public_key;
	while (tmp % phi_n != 1) {
		tmp += public_key;
		tmp %= phi_n;
		(*private_key)++;
	}
}


//encrypt an array of char (string) into an array of numbers, map one -> one.
void get_cipher(char *input, long public_key, long n, long *output, int len) {
	int i, j,chunk = 3, tid;
    clock_t start, end;
	#pragma omp parallel shared(input, output, public_key, n, chunk) private(i,j, tid, start, end)
	{
		// if(tid == 0){
		// 	clock_t start =  clock();
		// }
		#pragma omp for schedule(dynamic, chunk)
		for (i=0; i< len; i++) {
			long c = (long) input[i];
			for (j=1; j<public_key; j++) {
				c *= input[i];
				c %= n;

			}
			output[i] = c;
		}
		// if (tid == 0){
		// 	clock_t end =  clock();
    	// 	double time = ((double)(end - start)) / CLOCKS_PER_SEC;
    	// 	printf("encryption time %f\n", time);
		// 	printf("en%d\n", tid);
		// }

	}
	
}

//decrypt an array of numbers
void decrypt_cipher(long * input, long private_key, long n, char * output, int len) {
	int i, j, chunk = 3, nthreads, tid;
	clock_t start, end;
    
	#pragma omp parallel shared(input, output, private_key, n, chunk, nthreads) private(i, j, tid, start, end)
	{
		// nthreads = omp_get_num_threads();
		// tid = omp_get_thread_num();
		// if(tid == 0){
		// 	clock_t start =  clock();
		// }
		#pragma omp for schedule(dynamic, chunk)
		for (i=0; i<len; i++) {
			long int c = input[i];
			for (j = 1; j<private_key; j++) {
				c *= input[i];
				c %= n;
			}
			output[i] = (char) c;
		}
		// if (tid == 0){
		// 	clock_t end =  clock();
    	// 	double time = ((double)(end - start)) / CLOCKS_PER_SEC;
    	// 	printf("decryption time %f\n", time);
		// }
	}
	printf("num thread %d\n", nthreads);
	output[len] = '\0';
    
}



//literally, generate public and private key, rank can be seened as seed.
void generate_keys(int rank, long *public_key, long *private_key, long*n) {
	long primeNum[] = {5821, 5749, 223, 239, 241, 269, 263, 257, 251, 241, 271, 277, 307, 311, 1283, 1303, 1301, 1249, 1543, 53, 71, 103, 107, 109, 113, 127, 131, 97, 89, 1553, 1699, 1567, 2594, 2609, 2617, 2621, 2633, 2647, 2657, 2767, 2003, 2017, 2081,
	3001, 3023, 3037, 3109, 3163, 337, 101, 257, 211, 227, 229, 233,  331, 313, 103, 193, 293, 347, 127, 389, 83, 113, 281, 193, 197, 3221, 3191, 3361, 3371, 3529, 3533, 3539, 3671, 3673, 3677, 3793, 3821, 4729, 4831, 4073, 4001};
	int size_prime_list = sizeof(primeNum)/sizeof(long);
	srand(time(0)+rank);
	long p = primeNum[rand() %size_prime_list];
	srand(time(0)+rank+1);
	long q = primeNum[rand() %size_prime_list];
	*n = p*q;

	get_public_key(p, q, public_key);
	get_private_key(p, q, *public_key, private_key);
}

// int main()
// {
// 	long pub;
// 	long pri; 
// 	long n;
//     char * text = "that is bullshit, fuck this assignment, what is this, wtf";
//     long * cipher = (long*)malloc(strlen(text)*sizeof(long));
// 	long * buff = (long*)malloc(strlen(text)*sizeof(long));
//     char * text_before =  (char*)malloc(strlen(text)*sizeof(char));;
// 	generate_keys(3, &pub, &pri, &n);

//     get_cipher(text, pub, n, cipher, strlen(text));
// 	decrypt_cipher(cipher, pri, n, text_before, strlen(text));

//     printf("text before %s\n", text_before);
//     printf("public key %ld\n", pub);
//     printf("private key %ld\n", pri);
// 	printf("n %ld\n", n);
 
//     return 0;
// }