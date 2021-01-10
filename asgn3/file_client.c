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

int main(){
	struct sockaddr_in servaddr,cliaddr;
	// create socket
	int sockfd = socket(AF_INET,SOCK_STREAM,0);

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

	// connecting with server
	if(connect(sockfd,(const struct  sockaddr *)&servaddr, sizeof(servaddr)) <0){
		printf("Connection failed\n");
		return 0;
	}
	else{
		printf("connected to server\n");
	}
	char buff[c_buffer], f_name[max];

	//overwrite space with endlines for inialisation
	memset(buff,'\0' , sizeof(buff));
	//overwrite file name with 0s
	memset(f_name,'\0' , sizeof(f_name));


	printf("Enter the name of the file : ");
	scanf("%s",f_name);
	//printf("%s\n",f_name); for debugging

	// send file name
	if(send(sockfd,f_name,strlen(f_name),0)<0){
		printf("send() failed\n");
	}
	else{
		printf("file name sent\n");
	}

	//open/create file to write and give write permissions
	//int file_writer=open(f_name, O_RDWR|O_CREAT,0640);
	//if(file_writer<0){
	//	printf("File creation/opening failed\n");
	//}

	int rcv_size,w_count=0,no_bytes=0,file_writer;
	bool flag=true,exists=true;
	// loop until bytes of data recieved is greater than 0
	while((rcv_size=recv(sockfd,buff,sizeof(buff),0))>0){
		if(exists){
			file_writer=open(f_name, O_RDWR|O_CREAT,0640);
			if(file_writer<0){
				printf("File creation/opening failed\n");
			}
			exists=false;
		}
		// indicate end of string
		buff[rcv_size]='\0';
		//printf("%s\n", buff ); // for debugging
		// write data to the opened file
		int test=write(file_writer,buff,strlen(buff));
		//printf("%d\n",test ); // for debugging
		// count the number of words
		for (int k = 0; buff[k]!='\0'; ++k)
		{
			no_bytes++;
			char current=buff[k];
			bool flag1 = (current==','||current=='.'||current==':'||current==';'||current==' '||current=='\n');
			if(flag1){
				flag=true;
			}
			else if(flag && !flag1){
				w_count++;
				flag=false;
			}
		}
		memset(buff,'\0' , sizeof(buff));
	}
	close(file_writer);
	//close the file that we're writing too

	if(no_bytes==0){
		printf("File not Found, Connection closed\n");
	}
	// else print the statistics
	else{
		printf("\nFile transfer over.\n");
		printf("No of words assuming : %d\n",w_count);
		printf("No of bytes read: %d\n",no_bytes);
	}
	// close the socket
	close(sockfd);
	return 0;
}

