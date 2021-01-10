#include <arpa/inet.h> 
#include <errno.h> 
#include <netinet/in.h> 
#include <signal.h> 
#include <stdio.h> 
#include <stdlib.h> 
#include <strings.h> 
#include <sys/socket.h> 
#include <sys/types.h> 
#include <unistd.h>  
#include <string.h>
#include <stdbool.h> 

// Change port and buffer size
#define port 8181
#define MAXLEN 1024 

bool check_flag(int flag, char* error){
    if(flag<0){
        printf("%s \n", error);
        return false;
    }
    else{
        return true;
    }
}

int main() 
{
    // some definitions 
    int sockfd,n; 
    struct sockaddr_in servaddr;
    int servlen= sizeof(servaddr);
    // The domain name whose IP we're going to request 
    char buff[MAXLEN]="www.google.com";
    // overwrite servadd struct block with 0's
    memset(&servaddr, 0, sizeof(servaddr));
    // create socket
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if(!check_flag(sockfd,"Socket creation failed")){ 
        return 0;
    } 
    // Server parameters 
    servaddr.sin_family = AF_INET; 
    servaddr.sin_port = htons(port); 
    servaddr.sin_addr.s_addr = INADDR_ANY;     
    // Send Domain name
    int flag=sendto(sockfd, buff, strlen(buff),0,(struct sockaddr*)&servaddr,sizeof(servaddr));
    if(!check_flag(flag,"sendto failed")){ 
        return 0;
    } 
    printf("Domain sent: %s\n",buff); // for testing

    // Receive the IP address
    n = recvfrom(sockfd, buff, MAXLEN,0,(struct sockaddr*)&servaddr,&servlen); 
    buff[n]='\0';

    // Printing the received IP address
    printf("IP address received=%s\n", buff); 
    close(sockfd); 
    return 0; 
} 
