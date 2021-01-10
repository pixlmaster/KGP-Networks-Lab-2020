#include <stdio.h> 
#include <stdlib.h> 
#include <unistd.h> 
#include <string.h> 
#include <sys/types.h> 
#include <sys/socket.h> 
#include <arpa/inet.h> 
#include <netinet/in.h>
#include <stdbool.h> 

char* concat(const char *s1, const char *s2)
{
    char *result = malloc(strlen(s1) + strlen(s2) + 1); // +1 for the null-terminator
    // in real code you would check for errors in malloc here
    strcpy(result, s1);
    strcat(result, s2);
    return result;
}

int main(){
	int s;
	struct sockaddr_in servaddr;

	s = socket(AF_INET, SOCK_DGRAM, 0);
	if(s<0){
		perror("socket creation failed");
		exit(EXIT_FAILURE);
	}

	memset(&servaddr,0,sizeof(servaddr));

	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(8181);
	servaddr.sin_addr.s_addr = INADDR_ANY;

	int n;
	socklen_t len;
	char file1[20];
	char buffer[1024];
	FILE* filewriter;

	bool exit_flag=false;
	while(exit_flag==false){
		printf("Enter \"exit\" to close server and client \n");
		//printf("here\n");
		scanf("%s", file1);
		char* error_message = concat("NOTFOUND ", file1);
		//printf("there\n");
		if(strcmp(file1,"exit") == 0){
			//printf("entered\n");
			exit_flag=true;
		}
		//printf("%d\n", exit_flag);
		sendto(s, (const char *) file1, strlen(file1),0,
			(const struct sockaddr *) &servaddr, sizeof(servaddr));
		
		if(strcmp(file1,"exit") == 0){
			//printf("entered\n");
			return 0;
		}
		//printf("%s\n", file1 );

		n = recvfrom(s, (char *)buffer, 1024, 0,( struct sockaddr *) &servaddr, &len);
		buffer[n] = '\0';

		if(strcmp(buffer, error_message)==0){
			printf("FILE %s NOT FOUND\n", file1 );
			printf("Exiting....\n");
			return 0;	
		}

		//printf("%s\n", buffer );
		//printf("%d\n", strcmp(buffer, "HELLO\n") );

		if(strcmp(buffer, "HELLO\n")==0){
			//printf("file found\n");
			int i=1;
			filewriter = fopen("result.txt", "w+");

			char* request_word = concat("WORD", "0");
			i++;

			//sendto(s, (const char *) request_word, strlen(request_word),0,(const struct sockaddr *) &servaddr, sizeof(servaddr));
			while(strcmp(buffer,"END\n") != 0 && strcmp(buffer,"END") != 0){
				//printf("in loop\n");
				//char redundant[20];
				//scanf("%s", redundant);
				//j++;

				request_word = concat("WORD", "0");
				i++;
				
				sendto(s, (const char *) request_word, strlen(request_word),0,
				(const struct sockaddr *) &servaddr, sizeof(servaddr));

				n = recvfrom(s, (char *)buffer, 1024, 0, 
				( struct sockaddr *) &servaddr, &len);
				buffer[n] = '\0';

				//printf("%s\n", buffer);

				if(strlen(buffer) >=3 && buffer[0]=='E' && buffer[1]=='N' && buffer[2]=='D'){
					break;
				}

				fprintf(filewriter , "%s", buffer);
			}
			request_word="END";
			sendto(s, (const char *) request_word, strlen(request_word),0,
				(const struct sockaddr *) &servaddr, sizeof(servaddr));
			fclose(filewriter);
		}

	}

	close(s);
	return 0;
}
