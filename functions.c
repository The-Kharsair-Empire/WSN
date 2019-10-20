//Student Name: Jiayuan Chang
//Student ID: 28718038
#include "WSN.h"

//this files are some of the functions needed

void getIP(char *IP){ 

     int fd; 

     struct ifreq ifr; 

     fd = socket(AF_INET,SOCK_DGRAM,0); 

     ifr.ifr_addr.sa_family = AF_INET; 

     strncpy(ifr.ifr_name,"eth0",IFNAMSIZ-1); 

     ioctl(fd,SIOCGIFADDR,&ifr); 

     close(fd); 

     sprintf(IP,"%s",inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr) -> sin_addr)); 

}

char* substring(int start, int end, char a_string[]){
    char *buffer = (char*) malloc(sizeof(char)*(end-start+1));
    int i;
    int j = 0;
    for(i = start; i< end; i++){
        buffer[j++] = a_string[i];
    }
    buffer[j] = '\0';
    return buffer;
}

