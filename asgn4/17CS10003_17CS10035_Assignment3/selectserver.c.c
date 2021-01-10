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
#include <dirent.h>
#include <pthread.h>
#include <sys/wait.h>

// Macros
#define PORT 8181
#define MAXLEN 1024 
// function for covenience to check flag and print errors
bool check_flag(int flag, char* error){
    if(flag<0){
        printf("%s \n", error);
        return false;
    }
    else{
        return true;
    }
}

// Start of main program
int main() 
{ 
    // some definitions
    int sockfdt,sockfdu; 
    unsigned char buffer[MAXLEN]; 
    socklen_t clilen;      
    struct sockaddr_in cliaddr, servaddr;
    // overwriting servaddr and cliaddr memory blocks with 0 for initilisation
    memset(&servaddr,0,sizeof(servaddr));
    memset(&cliaddr,0,sizeof(cliaddr)); 

    // creating TCP socket
    sockfdt = socket(AF_INET, SOCK_STREAM, 0); 
    if(!check_flag(sockfdt,"Socket creation failed.")){
        return 0;
    } 

    // server parameters
    servaddr.sin_family = AF_INET; 
    servaddr.sin_addr.s_addr = INADDR_ANY; 
    servaddr.sin_port = htons(PORT); 
  
    // Binding to local address 
    int flag = bind(sockfdt, (struct sockaddr*)&servaddr, sizeof(servaddr));
    if(!check_flag(sockfdt,"Bind failed.")){
        return 0;
    } 
    listen(sockfdt, 5); 
    // TCP socket created
  
    //Create UDP socket
    sockfdu = socket(AF_INET, SOCK_DGRAM, 0); 
    flag = bind(sockfdu, (struct sockaddr*)&servaddr, sizeof(servaddr));
    if(!check_flag(flag,"Bind Failed.")){
        return 0;
    }
    // UDP socket created

    // Initialise fdset to NULL set 
    fd_set fdset; 
    FD_ZERO(&fdset); 
  
    // set limit according to the order of udp socket and tcp socket
    int limit=sockfdu+1; 
    if(sockfdu<sockfdt){
        limit=sockfdt;
    }

    while(1) { 
        // Insert sockfdt and sockfdu in the set fdset 
        FD_SET(sockfdt, &fdset);
        FD_SET(sockfdu, &fdset); 
        // select and replace the fdset set with the one which is ready for the operation 
        select(limit, &fdset, NULL, NULL, NULL); 
  
        //check if tcp socket is a subset of fdset
        bool subsett=FD_ISSET(sockfdt, &fdset);
        if (subsett) {
            // TCP part 
            printf("TCP socket running.\n");
            clilen = sizeof(cliaddr);
            // accept the incoming connection
            int newsockfdt = accept(sockfdt, (struct sockaddr*)&cliaddr, &clilen); 
            // Creating new process using fork
            if (fork() == 0) { 
                // Child process
                close(sockfdt);  // Closing original sockfdt      
                // DO tcp stuff
                // receive the dir name
                char buffer[MAXLEN];
                bzero(buffer, MAXLEN);
                int drec=recv(newsockfdt,buffer,sizeof(buffer),0);
                buffer[drec]='\0';
                // define some directory constants
                char current[]="image/";
                strcat(current,buffer);
                printf("Trying to get file from directory=%s\n", current);  // for testing
                DIR *d;
                struct dirent *dir;
                d= opendir(current);

                if(d){
                    while(1){
                        dir = readdir(d);
                        // if no files in directory
                        if(dir==NULL){
                            break;
                        }
                        // get the dir name
                        char *dirname=dir->d_name;
                        if(dirname[0] <='9' && dirname[0]>='0'){
                            // get the current working directory
                            char current1[MAXLEN];
                            strcpy(current1,current);
                            strcat(current1,"/");
                            strcat(current1,dirname);
                            printf("Transferring image-%s\n",current1 );
                            // get the file name
                            char fpath[MAXLEN];
                            strcpy(fpath,current1);
                            // open the file
                            FILE *fp= fopen(fpath,"r");
                            // wait some time to avoid discrepancies
                            sleep(2);
                            if(fp){
                                while(1){
                                    // clean buffer
                                    bzero(buffer, MAXLEN);
                                    // read MAXLEN chunks of data and send
                                    int numread = fread(buffer,1,MAXLEN,fp);
                                    if(numread>0){
                                        send(newsockfdt, buffer, sizeof(buffer),0);
                                    }
                                    if(numread<MAXLEN){
                                        if(feof(fp)){
                                            printf("image trasferred- %s\n", current1);
                                            break;
                                        }
                                        if(ferror(fp)){
                                            printf("Error in Reading\n");
                                            break;
                                        }
                                    }
                                }
                                sleep(2);
                                // send delimited of F_DEL after every image transfer
                                char *delim="F_DEL";
                                send(newsockfdt,delim,sizeof(delim),0);
                            }
                            // sleep to avoid discrepancies
                            sleep(2);
                        }
                    }
                    // close the open directory
                    closedir(d);
                    // send END to mark end of message
                    char *err="END\n";
                    send(newsockfdt, err,strlen(err),0);
                }
                else{
                    // if directory does not exist.
                    printf("directory does not exist. closing connection...\n");
                    char *err="END\n";
                    send(newsockfdt, err,strlen(err),0);
                }
                close(newsockfdt); 
                exit(0); 
            } 
            // close the socket
            close(newsockfdt); 
        } 
        //check if udp socket is a subset of fdset
        bool subsetu=FD_ISSET(sockfdu, &fdset); 
        //printf("%d\n", subsetu ); // for testing
        if (subsetu) { 
            printf("UDP socket is running...\n");
            ssize_t n;
            clilen = sizeof(cliaddr); 
            // clean buffer
            memset(buffer,0,sizeof(buffer)) ;
            // Receive domain name from client
            n = recvfrom(sockfdu, buffer, sizeof(buffer), 0,(struct sockaddr*)&cliaddr, &clilen);
            buffer[n]='\0' ;
            printf("Recieved domain Name: %s\n",buffer); 
            char *name;
            // Using gethostbyname to get the ip of the domain
            struct hostent *address=gethostbyname(buffer);
            name = inet_ntoa(*((struct in_addr*)address->h_addr_list[0]));            
            strcpy(buffer,name);
            printf("IP sent: %s\n",buffer);
            // send the IP to client
            sendto(sockfdu, buffer, sizeof(buffer), 0,(struct sockaddr*)&cliaddr, sizeof(cliaddr)); 
        } 
    } 
    return 0;
}
