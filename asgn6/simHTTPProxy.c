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
#include <signal.h>
#include <errno.h>

#define MAXLEN 65535
#define MAXURL 100
#define MAXREQ 100
#define STDIN 0

#include<stdio.h>
#include<string.h>

char* pasrsebuff(char* mainstr,char * substr,char* newstr)
{
    int lenmain,lensub,i,j,lennew,startindex=-1,limit,c;
    lenmain=strlen(mainstr);
    lensub=strlen(substr);
    lennew=strlen(newstr);
    char *result=(char*)malloc(sizeof(char)*(lenmain+100));
    for(i=0;i<lenmain;i++)
        {
        if(lenmain-i>=lensub&&*(mainstr+i)==*(substr))
            {
            startindex=i;
            for(j=1;j<lensub;j++)
                if(*(mainstr+i+j)!=*(substr+j))
                    {
                    startindex=-1;
                    break;
                    }
            if(startindex!=-1)
                break;
            }
        }
    limit=(startindex==-1)?lenmain:startindex;
    for(i=0;i<limit;i++)
        *(result+i)=*(mainstr+i);
    if(startindex!=-1)
        {
        for(j=0;j<lennew;j++)
            *(result+i+j)=*(newstr+j);
        c=i+lensub;
        i=i+j;
        for(;c<lenmain;c++,i++)
            *(result+i)=*(mainstr+c);
        }
    *(result+i)='\0';
    return result;
}

int parse_header(char *buffer , char* host , char* port){
    char *data = strstr(buffer, "\r\n\r\n");
    char header[MAXLEN];
    if(data!=NULL)
    { 
        int itr=0;
        while(1){
            if(buffer[itr]=='\r' && buffer[itr+1]=='\n' && buffer[itr+2] =='\r' && buffer [itr+3]=='\n' ){
                header[itr]='\0';
                break;
            }
            header[itr]=buffer[itr];
            itr++;
        }           
    }
    else{
        return -1;
    }
    char url[MAXURL];
    
    int itr=0;
    if(header[itr]=='P' && header[itr+1]=='O' && header[itr+2]=='S' && header[itr+3]=='T'){
        itr+=5;
    }
    else if(header[itr]=='G' && header[itr+1]=='E' && header[itr+2]=='T'){
        itr+=4;
    }
    else{
    	return -1;
    }


    int pos=0;
    while(1){
        if(header[itr+1]=='H' && header[itr+2]=='T' && header[itr+3]=='T' && header[itr+4]=='P'){
            url[pos] = '\0';
            break;
        }
        url[pos]=header[itr];
        pos++;
        itr++;
    }
    itr=0;
    pos=0;
    int flag=0;
    while(1){
        if(flag ==0 && header[itr]=='H' && header[itr+1]=='o' && header[itr+2]=='s' && header[itr+3]=='t' && header[itr+4]==':'){
            itr+=6;
            //printf("%c\n",header[itr] );
            flag=1;
        }
        if(flag==1){
            if(header[itr]=='\n' || header[itr]==' '){
                host[pos]='\0';
                break;
            }
            host[pos]=header[itr];
            pos++;
        }
        itr++;
    }
    //printf("%d\n",strlen(host) );
    printf("%s\n",url);
    char final_url[]="";
    char *to1=strtok(url,"/");
    strcat(final_url,to1);
    strcat(final_url,"//");
    to1=strtok(NULL,"/");
    strcat(final_url,to1);
    strcat(final_url,"/");
    printf("CHECK to1 - %s\n", final_url);
    if(strcmp(final_url,"http://detectportal.firefox.com/")==0){
        printf("%s\n",buffer );
        return -1;
    }

    //printf("2______\n%s\n______\n",buffer );
    strcpy(buffer,pasrsebuff(buffer,final_url,"/"));
    //printf("3______\n%s\n______\n",buffer );
    host[strlen(host)-1]='\0';
    char *token = strtok(host,":");
    while(token!=NULL){
        strcpy(port,token);
        token = strtok(NULL, ":");
    }
    char def[] = "80";
    if(strcmp(port,host)==0 || port==NULL){
        strcpy(port,def);
    }
    return 0;
}

void ipfromhost(char *host, char* ip){
    struct hostent *he;
    struct in_addr addr;
    //printf("%d\n",strlen("cbdhfkmnsltvwrxz.neverssl.com") );
    he = gethostbyname( host );
    if(he!=NULL){
        addr = *((struct in_addr*)he->h_addr_list[0] );
        strcpy(ip,inet_ntoa(addr));
        return;
    }
    printf("couldnt get ip\n");
    return;
}


bool check_flag(int flag, char* error){
    if(flag<0){
        printf("%s \n", error);
        return false;
    }
    else{
        return true;
    }
}

int max_socket(int x1,int x2,int x3){
    int m = x1>x2?x1:x2;
    return x3>m?x3:m;
}

int main(int argc, char const *argv[]){

    if(argc!=2){
        printf("Parameters not enter correctly\n Usage- ./a.out client_port kgp_proxy kgp_port \n");
        return 0;
    }

    printf("Proxy running on port %s.\n", argv[1]);

    int sockfdi;
    int client_socket[MAXREQ], out_socket[MAXREQ];
    char in_buffer[MAXREQ][MAXLEN];


    for (int i = 0; i < MAXREQ; ++i)
    {
        client_socket[i]=0;
        out_socket[i]=0;
    }
    unsigned char buffer[MAXLEN];
    socklen_t clilen;
    struct sockaddr_in cliaddr, servaddr, out_proxy;
    // overwriting servaddr and cliaddr memory blocks with 0 for initilisation
    memset(&servaddr,0,sizeof(servaddr));
    memset(&cliaddr,0,sizeof(cliaddr));
    memset(buffer,0, sizeof(buffer));


    // creating a socket for the client
    sockfdi = socket(AF_INET, SOCK_STREAM, 0);    
    if(!check_flag(sockfdi,"socket creation failed")){
        return 0;
    }
    // making client socket not blocking
    int flag = fcntl(sockfdi, F_SETFL, O_NONBLOCK);
    if(!check_flag(flag,"Could not make port non-blocking")){
        return 0;
    }

    // server parameters
    servaddr.sin_family = AF_INET; 
    servaddr.sin_addr.s_addr = INADDR_ANY; 
    servaddr.sin_port = htons(atoi(argv[1])); 
    // enable reusability of ports
    int enable = 1;
    flag = setsockopt(sockfdi, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int));
    if(!check_flag(flag,"setting sockets to be reusable failed")){
        return 0;
    }
    // bind the server
    flag = bind(sockfdi, (struct sockaddr*)&servaddr, sizeof(servaddr));
    if(!check_flag(flag,"Bind failed")){
        return 0;
    }
    // Listen for incoming connection
    listen(sockfdi,MAXREQ);

    // create 2 sets, one fore reading, one for writing
    fd_set readfds,sendfds;
    FD_ZERO(&readfds);
    FD_ZERO(&sendfds);
    // tracks number of active connections
    int active_conn=0;
    // define the signal
    signal(SIGPIPE, SIG_IGN);
    // Main loop
    while(1)
    {   
        // maintain only 100 active connections at a time
        if(active_conn<MAXREQ){

            // initiliaze set
            FD_ZERO(&readfds);
            FD_ZERO(&sendfds);

            // add sockfdi and STDIN in read set
            FD_SET(sockfdi, &readfds);
            FD_SET(STDIN, &readfds);
            // track the max return of socket
            int max_num=sockfdi;
            memset(buffer,0, sizeof(buffer));

            // adding active connections back to read set
            for (int i = 0; i < MAXREQ; ++i)
            {
                if(client_socket[i]>0){
                    FD_SET(client_socket[i], &readfds);
                    FD_SET(client_socket[i], &sendfds);
                }
                if(out_socket[i]>0){
					FD_SET(out_socket[i], &readfds);
                    FD_SET(out_socket[i], &sendfds);
                }
                max_num=max_socket(max_num,client_socket[i],out_socket[i]);
            }

            int curr = select(max_num+1, &readfds,&sendfds,NULL,NULL);
            //printf("selected\n");

            if(curr<=0)continue;
            // if STDIN is in subset of readfd
            if(FD_ISSET(STDIN, &readfds)){
                //printf("here\n");
                memset(buffer,0,sizeof(buffer));
                // read from STDIN
                int a =read(STDIN,buffer, sizeof(buffer));
                buffer[a]='\0';
                if(strncmp(buffer, "exit", 4 ) == 0){
                    printf("Exit Command Recieved\n");
                    for(int i = 0;i< MAXREQ;i++ ){
                        // close active connections
                        if( client_socket[i]  != 0){
                            close(client_socket[i]);
                            close(out_socket[i]);
                        }
                    }
                    // close master sockets
                    close(sockfdi);
                    return 0;
                }
            }
            // if client socket is member of read
            if(FD_ISSET(sockfdi, &readfds)){
                //printf("accepting connections\n");
                clilen = sizeof(cliaddr);
                int new_socket= accept(sockfdi, (struct sockaddr *)&cliaddr, &clilen);
                active_conn++;
                if(!check_flag(new_socket,"accept failed")){
                    continue;
                }

                flag = fcntl(new_socket, F_SETFL, O_NONBLOCK);
                if(!check_flag(flag, "making ports non blocking failed")){
                	return 0;
                }

                printf("connection accepted from: %s:%d \n",inet_ntoa(cliaddr.sin_addr),ntohs(cliaddr.sin_port));
                memset(buffer,0, sizeof(buffer));
                int a=  read(new_socket, buffer, sizeof(buffer));
                if(a<=0){
                	printf("Connection closed because no data read from port\n");
                	close(new_socket);
                    active_conn--;
                	memset(buffer,0,sizeof(buffer));
                	continue;
                }
                buffer[a]= '\0';
                int tempsize = a;
                //printf("1______\n%s\n______\n",buffer );
                char host[MAXURL];
                char ip[MAXURL];
                char port[MAXURL];
                if(parse_header(buffer, host, port)==-1){
                	printf("dropping because first req was not GET or POST\n");
                	close(new_socket);
                    active_conn--;
                	memset(buffer,0,sizeof(buffer));
                	continue;
                }
                ipfromhost(host, ip);
                printf("%s %s\n",ip,port);

                out_proxy.sin_family = AF_INET;
                out_proxy.sin_addr.s_addr = inet_addr(ip);
                out_proxy.sin_port = htons(atoi(port));

                int send_socket = socket(AF_INET, SOCK_STREAM, 0);
                if(!check_flag(send_socket," ip not found")){
                	close(new_socket);
                    active_conn--;
                	memset(buffer,0,sizeof(buffer));
                	continue;
                }

                flag = connect(send_socket, (struct sockaddr *)&out_proxy,sizeof(out_proxy));
                
                if(!check_flag(flag,"connection to out proxy failed")){
                	printf("%s\n",strerror(errno) );
                	close(new_socket);
                	close(send_socket);
                    active_conn--;
                	memset(buffer,0,sizeof(buffer));
                	continue;
                }
                printf("connection created \n");
                flag = fcntl(send_socket, F_SETFL, O_NONBLOCK);
                if(!check_flag(flag, "making ports non blocking failed")){
                  return 0;
                }
                //printf("%s\n",buffer );
                int sent=0;
                while(sent<tempsize){
                    flag = send(send_socket, buffer, a, 0 );
                    if(flag<=0){
                            switch(errno){
                            case EPIPE:
                                printf("closing connection with sockfd= %d \n",new_socket);
                                close(new_socket);
                                close(send_socket);
                                active_conn--;
                        }                            // continue;
                    }
                    sent+=flag;
                    if(errno == EPIPE) continue;
                }

                // add created connection to the list
                for (int i = 0; i < MAXREQ; ++i)
                {
                    if(client_socket[i]==0){
                        client_socket[i] = new_socket;
                        out_socket[i] = send_socket;
                        break;
                    }
                }
                continue;
            }
            // check open connections and transfer data
            for (int i = 0; i < MAXREQ; ++i)
            {
                // if client sends a packet
                if(FD_ISSET(client_socket[i], &readfds) && FD_ISSET(out_socket[i], &sendfds)){
                    //printf("reading from client\n");
                    // read from client socket
                    memset(buffer,0,sizeof(buffer));
                    int a=  read(client_socket[i], buffer, sizeof(buffer));
                    //char host[MAXURL];
                    //char port[MAXURL];
                    //if(parse_header(buffer, host, port)==-1){
                    //    printf("dropping because not GET or POST\n");
                    //    memset(buffer,0,sizeof(buffer));
                    //    continue;
                    //}
                    if(a<=0){
                        printf("Connection closed because no data read from port\n");
                        close(client_socket[i]);
                        client_socket[i]=0;
                        close(out_socket[i]);
                        out_socket[i]=0;
                        active_conn--;
                        memset(buffer,0,sizeof(buffer));
                        continue;
                    }
                    buffer[a]= '\0';
                    int sent=0;
                    while(sent<a){
                        flag = send(out_socket[i], buffer, a, 0 );
                        if(flag<=0){
                                switch(errno){
                                case EPIPE:
                                    printf("closing connection with sockfd= %d \n",client_socket[i]);
                                    close(client_socket[i]);
                                    client_socket[i]=0;
                                    close(out_socket[i]);
                                    out_socket[i]=0;
                                    active_conn--;
                            }                            // continue;
                        }
                        sent+=flag;
                        if(errno == EPIPE) continue;
                    }
                }
                // when kgp-proxy sends something to user
                if(FD_ISSET(out_socket[i], &readfds) && FD_ISSET(client_socket[i], &sendfds)){
                    //printf("receiving something from site\n");
                    // read from kgp-proxy socket
                    memset(buffer,0,sizeof(buffer));
                    int a = read(out_socket[i], buffer, sizeof(buffer));

                    if(a<=0){
                        printf("Connection closed because no data read from port\n");
                        close(client_socket[i]);
                        client_socket[i]=0;
                        close(out_socket[i]);
                        out_socket[i]=0;
                        active_conn--;
                        memset(buffer,0,sizeof(buffer));
                        continue;
                    }
                    buffer[a]= '\0';
                    //printf("4______\n%s\n______\n",buffer );
                    // send to client
                    int sent =0;
                    while(sent<a){
                        flag = send(client_socket[i], buffer, a, 0 );
                        if(flag<=0){
                                switch(errno){
                                case EPIPE:
                                    printf("closing connection with sockfd= %d \n",client_socket[i]);
                                    close(client_socket[i]);
                                    client_socket[i]=0;
                                    close(out_socket[i]);
                                    out_socket[i]=0;
                                    active_conn--;
                            }                            // continue;
                        }
                        sent+=flag;
                        if(errno == EPIPE) continue;
                    }
                }
            }

        }
    }
}