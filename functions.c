
#include "WSN.h"

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

void reverse(char *str, int len) 
{ 
    int i=0, j=len-1, temp; 
    while (i<j) 
    { 
        temp = str[i]; 
        str[i] = str[j]; 
        str[j] = temp; 
        i++; j--; 
    } 
} 

int intToStr(int x, char str[], int significantNum) 
{ 
    int i = 0; 
    while (x) 
    { 
        str[i++] = (x%10) + '0'; 
        x = x/10; 
    } 
    while (i < significantNum) 
        str[i++] = '0'; 
  
    reverse(str, i); 
    str[i] = '\0'; 
    return i; 
} 

void ftoStr(double n, char *res, int significantNum) 
{  
    int intpart = (int)n; 
    float fpart = n - (float)intpart;  
    int i = intToStr(intpart, res, 0); 
    if (significantNum != 0) 
    { 
        res[i] = '.';  
        fpart = fpart * pow(10, significantNum); 
        intToStr((int)fpart, res + i + 1, significantNum); 
    } 
} 