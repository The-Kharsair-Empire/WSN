#include<stdio.h>
#include<math.h>
#include<stdlib.h>
#include<string.h>
#include <time.h>

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
void get_public_key(long p, long q, long *n, long * phi_n, long * public_key){
    *phi_n = (p-1)*(q-1);
    *n = p*q;
    *public_key = 3 ;//(*n)/2
    while (*public_key < *phi_n){
        if (gcd((*public_key), (*phi_n)) == 1) break;
        else (*public_key)++;
    }
}

// calculate private key based on generated public key
//simply: private_key = (1 + (2*phi_n))/public_key, but have to go through key expansion;
void get_private_key(long p, long q, long * phi_n, long * public_key, long * private_key){
    *private_key = 1;
    long temp = *public_key;
    while (temp % *phi_n != 1){
        temp = (temp + *public_key) % *phi_n;
        (*private_key)++;
    }    
}

//encrypt one single char to a cipher number using public key
void encrypt_one(char * a_char, long public_key, long * cipher, long n){
    long ascii_code = (int) *a_char;
    *cipher = pow(ascii_code, public_key);
    *cipher = fmod(*cipher, n);
}

//decrypt one single cipher number back to single char using private key
void decrypt_one(long *cipher, long private_key, char * a_char, long n){
    *a_char = (char) fmod(pow(*cipher, private_key), n);
}

//encrypt an array of char (string) into an array of numbers, map one -> one.
void get_cipher(char text[], long *n, long *public_key, long cipher[], int size){
    int i, chunk = 3;
    clock_t start =  clock();
   
    #pragma omp for schedule(dynamic, chunk)
    for(i = 0; i < size; i++){
        encrypt_one(&text[i], *public_key, &cipher[i],*n);
    }
    clock_t end =  clock();
    double time = ((double)(end - start)) / CLOCKS_PER_SEC;
    printf("encryption time %f\n", time);

}

//decrypt an array of numbers
void decrypt_cipher(char text[], long *n, long *private_key, long cipher[], int size){
    int i, chunk = 3;
    clock_t start =  clock();

    for (i = 0; i < size; i++){
        decrypt_one(&cipher[i], *private_key, &text[i],*n);
    }
    
    clock_t end =  clock();
    double time = ((double)(end - start)) / CLOCKS_PER_SEC;
    printf("decryption time %f\n", time);
}

//literally, generate public and private key, rank can be seened as seed.
void generate_key(int rank, long *public_key, long *private_key, long *n){
    long primes[] = {53, 233, 223, 239, 241, 269, 263, 257, 251, 241, 271, 277, 307, 311, 331, 313, 103, 193, 71, 103, 107, 109, 113, 127, 131, 97, 89, 337, 101, 293, 347, 127, 389, 83, 113, 281, 193, 197, 257, 211, 227, 229};

    long p = primes[rank*2];
    long q = primes[rank*2+1];
    long phi_n;

    get_public_key(p, q, n, &phi_n, public_key);
    get_private_key(p, q, &phi_n, public_key, private_key);
}

int main()
{
    char * text = "froasdadwdevertinaasdaskalsmdklamdlkmakldasdasdahdbajhasdasdaddbajhbdjabdbasdbajbdasaddasnkdjnaksjdnajknsdkjandknakjsndankjsdnkajnsdknkansdkjnakdnaksndkjandkjansdkjndkjnkajndkjwnadkjwndkjawndawkjdnanwkjdnakjnwdajfkandkwdnhjfabdknjalkwnasdasdsdasdaaaaaaaaaaaaaaaawdwdandkjwndkjankdw";
    long * cipher = (long*)malloc(strlen(text)*sizeof(long));
    char * text_before =  (char*)malloc(strlen(text)*sizeof(char));;
    long public_key;
    long private_key;
    long n;

    // printf("%s\n", text_before);
    generate_key(3, &public_key, &private_key, &n);
    get_cipher(text, &n, &public_key, cipher, strlen(text));
    decrypt_cipher(text_before, &n, &private_key, cipher, strlen(text));
    printf("%s\n", text_before);


    return 0;
}