#include <arpa/inet.h> 
#include <errno.h> 
#include <netinet/in.h> 
#include <signal.h> 
#include <stdio.h> 
#include <stdlib.h> 
#include <string.h> 
#include <sys/socket.h> 
#include <sys/types.h> 
#include <unistd.h> 
#include <netdb.h>
#include <stdbool.h>
#include <fcntl.h> 
#include <sys/stat.h> 

// Some macros
#define PORT 8181
#define MAXLEN 1024

// function for convenience, checks flag and prints error message accordingly
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
    int sockfd; 
    char buffer[MAXLEN];  
    struct sockaddr_in servaddr; 
  
    int n, len; 
    // Creating TCP socket
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) { 
        printf("Failed to create socket!\n"); 
        exit(0); 
    } 
    
    // initiliasing servadd block with all 0s
    memset(&servaddr, 0, sizeof(servaddr)); 
  
    // Server information 
    servaddr.sin_family = AF_INET; 
    servaddr.sin_port = htons(PORT); 
    servaddr.sin_addr.s_addr = INADDR_ANY; 
    //if connection fails 
    bool flag= connect(sockfd,(struct sockaddr*)&servaddr,sizeof(servaddr));
    if (!check_flag(flag,"Connection Failed.")){ 
        return 0;
    }

    char dir[MAXLEN];
    printf("Enter directory name\n");
    scanf("%[^\n]",dir);
    flag = send(sockfd, dir, strlen(dir),0);
    // if send fails
    if(!check_flag(flag,"Sending dir name failed.")){
        return 0;
    }
    // open temp file to write to
    int file_writer=open("temp.txt", O_WRONLY|O_CREAT,0640);
    if(!check_flag(file_writer,"File creation/opening failed")){
      return 0;
    }
    // recieve packets until it can and write it to temp file
    int rcv_size;
    while((rcv_size=recv(sockfd,buffer,sizeof(buffer),0))>0){
        buffer[rcv_size]='\0';
        write(file_writer, buffer, strlen(buffer));
        memset(buffer, '\0',sizeof(buffer));
    }
    //after this, search for the the instances of "F_DEL" until you encounter substring "END"r
    system("tr -dc '_A-Za-z0-9' <temp.txt >temp2.txt");
    //system("rm -f temp.txt"); //for testing

    FILE* file;
    file = fopen("temp2.txt", "r");

    if(file==NULL)
    {
        printf("Error in reading file!");
        return 0;
    }

    int count = 0;

    char ch1='a';
    char ch2='b';
    char ch3=fgetc(file);
    char ch4=fgetc(file);
    char ch5;
    // count number of images based on delimiter from temp
    while((ch5 = fgetc(file))!=EOF)
    {
        if(ch5=='D' && ch4=='N' && ch3=='E')
        {
            break;
        }
        else if(ch5=='L' && ch4=='E' && ch3=='D' && ch2=='_' && ch1=='F')
        {
            count++;
        }

        ch1=ch2;
        ch2=ch3;
        ch3=ch4;
        ch4=ch5;
    }
    // if there are no count then directory must not exist
    if(count<=0){
        printf("directory does not exist. Exiting...\n");
        return 0;
    }
    // report number of images
    printf("NUMBER of images: %d\n", count);
    fclose(file);
    close(file_writer);
    system("rm -f temp2.txt");
    system("rm -f temp.txt");

    printf("completed \n");
    close(sockfd); 
    
    
}
