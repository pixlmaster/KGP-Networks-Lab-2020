#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h> 
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h> 
#include <sys/stat.h>
#include <unistd.h>

// buffer sizes
#define c_buffer 10
#define s_buffer 10
#define max 100
#define max_conn 5

int main(){

	struct sockaddr_in servaddr, cliaddr;
	// create socket
	int sockfd = socket(AF_INET,SOCK_STREAM, 0);
	// error if unable to create socket
	if(sockfd<0){
		perror("socket creation failed");
		exit(EXIT_FAILURE);
	}
	// filling memory blocks with 0
	memset(&servaddr,0 , sizeof(servaddr));
	memset(&cliaddr, 0, sizeof(cliaddr));
	// server specifications
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = INADDR_ANY;
	servaddr.sin_port = htons(8181);
	// bind the socket
	if( bind(sockfd, (const struct sockaddr *) &servaddr,sizeof(servaddr))<0){
		perror("bind failed");
		exit(EXIT_FAILURE);
	}
	printf("server running...\n");
	// wait for connections , pop from queue
	listen(sockfd, max_conn);
	char f_name[max];
	// overwriting file name with endlines for initilisation
	memset(f_name,'\0' , sizeof(f_name));
	char buff[s_buffer];

	while(1)
	{
		socklen_t clilen = sizeof(cliaddr);
		// acceot the incoming connection
		int newsockfd = accept(sockfd, (struct sockaddr *) &cliaddr, &clilen);
		// if cannot accept print error
		if(newsockfd<0){
			printf("ERROR on accepting\n");
			return 0;
		}
		// receive the file name
		recv(newsockfd, f_name, max, 0);
		// if client entered exit, close server
		if(strncmp(f_name,"exit",4)==0){
			printf("exiting server...\n");
			return 0;
		}
		printf("\nFile name received %s\n", f_name );
		// open the file
		int file_reader = open(f_name,O_RDONLY);
		// print error and close connection if file does not exist
		if(file_reader<0){
			printf("File does not exist\n");
			close(newsockfd);
		}
		int count;
		// read and send packets
		while((count=read(file_reader,buff,s_buffer))>0){
			buff[count]='\0';
			//printf("%s\n", buff ); // for debugging
			send(newsockfd,buff,strlen(buff),0);
		}
		// close connection
		close(newsockfd);
	}
	//clsoe socket
	close(sockfd);
	return 0;
}