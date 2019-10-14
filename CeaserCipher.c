#include <omp.h> 
#include <stdio.h> 
#include <stdlib.h> 
#include <string.h>
#include <math.h>
#include <time.h>

void encrypt(int key_shift, char* text){

}

void xor(char text[], char key[]){
    int i = 0;
    for (; i < strlen(text); i++) {
        *(text+i) = text[i] ^ key[i % strlen(key)];
    }
}

int main(void){
    char text[] = "key";
    char key[] = "KEY";
    xor(text, key);
    printf("%s|end\n", text);
    
    return 0;
}
