#include<stdio.h>
#include<unistd.h>
#include<stdlib.h>
#include<string.h>
#include<arpa/inet.h>
#include<netinet/in.h>
#include <stdbool.h>

#include "rsocket.h"

#define USER2 10003

bool check_flag(int flag, char* error){
    if(flag<0){
        printf("%s \n", error);
        return false;
    }
    else{
        return true;
    }
}

int main(int argc, char *argv[])
{
    char input[100];

    struct sockaddr_in user2, user1;
    user2.sin_family = AF_INET;
    user2.sin_addr.s_addr = INADDR_ANY;
    user2.sin_port = htons(USER2);

    int sockfd = r_socket(AF_INET, SOCK_MRP, 0);
    if(!check_flag(sockfd,"Socket creation failed"))
    {
        exit(EXIT_FAILURE);
    }

    int flag = r_bind(sockfd, (struct sockaddr *)&user2, sizeof(user2)); 
    if(!check_flag(flag,"Bind failed"))
    {
        exit(EXIT_FAILURE);
    }
    
    while(1)
    {
        socklen_t size;
        int te = r_recvfrom(sockfd, input, 100, 0, (struct sockaddr *)&user1, &size);
        input[te] = '\0';
        printf("%s : From %s:%d\n", input, inet_ntoa(user1.sin_addr), ntohs(user1.sin_port));
    }
    r_close(sockfd);
}