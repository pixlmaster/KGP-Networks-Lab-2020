#include<stdio.h>
#include<unistd.h>
#include<stdlib.h>
#include<string.h>
#include<arpa/inet.h>
#include<netinet/in.h>
#include <stdbool.h>

#include "rsocket.h"

#define USER2 10003
#define USER1 10002

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
    printf("enter a string:\n");
    scanf("%s", input);
    int n = strlen(input);

    struct sockaddr_in user1, user2;
    user1.sin_family = AF_INET;
    user1.sin_addr.s_addr = INADDR_ANY;
    user1.sin_port = htons(USER1);

    user2.sin_family = AF_INET;
    user2.sin_addr.s_addr = INADDR_ANY;
    user2.sin_port = htons(USER2);

    int sockfd = r_socket(AF_INET, SOCK_MRP, 0);
    if(!check_flag(sockfd,"Socket creation failed"))
    {
        exit(EXIT_FAILURE);
    }

    int flag = r_bind(sockfd, (struct sockaddr *)&user1, sizeof(user1));

    if(!check_flag(flag,"Bind failed"))
    {
        exit(EXIT_FAILURE);
    }

    for(int i=0;i<n;i++)
    {
        r_sendto(sockfd, input + i, 1, MSG_DONTWAIT, (struct sockaddr *)&user2, sizeof(user2));
        usleep(300000);
    }
    //printf("closing socket...\n");
    r_close(sockfd);
}