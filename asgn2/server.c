#include <stdio.h> 
#include <stdlib.h> 
#include <unistd.h> 
#include <string.h> 
#include <sys/types.h> 
#include <sys/socket.h> 
#include <arpa/inet.h> 
#include <netinet/in.h>
#include <stdbool.h> 
#include <string.h>

#define MAXLINE 1024 

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
    struct sockaddr_in servaddr, cliaddr;

	s = socket(AF_INET, SOCK_DGRAM, 0);
    if ( s < 0 ) { 
        perror("socket creation failed"); 
        exit(EXIT_FAILURE); 
    }

    memset(&servaddr , 0 ,sizeof(servaddr));
    memset(&cliaddr, 0, sizeof(cliaddr));

    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = INADDR_ANY;
    servaddr.sin_port = htons(8181);

    if(bind(s, (const struct sockaddr *)&servaddr, sizeof(servaddr)) <0){
    	perror("bind failed");
    	exit(EXIT_FAILURE);
    }

    printf("\n server running \n");

    int n;
    socklen_t len;
    char buffer[MAXLINE];

    len = sizeof(cliaddr);
 
    //printf("%s\n", buffer);
    FILE* filereader;
    bool exit_flag=false;

    while(exit_flag==false){
    	printf("...\n");
    	 n = recvfrom(s, (char *)buffer, MAXLINE, 0, 
			( struct sockaddr *) &cliaddr, &len);
    	 buffer[n] = '\0';

    	 //printf("%s\n", buffer );
    	 
    	 if(strcmp(buffer,"exit") == 0){
    	 	exit_flag=true;
    	 	break;
    	 }
  	    
  	    filereader = fopen( buffer, "r" );
    	
    	if(filereader == NULL){
    		//printf("File not found\n");
    		
    		char* error_message = concat("NOTFOUND ", buffer);
    		
    		sendto(s, (const char *) error_message, strlen(error_message),0,
			(const struct sockaddr *) &cliaddr, sizeof(cliaddr));
   	 	}
   	 	else{
   	 		//printf("File found\n");
   	 		
   	 		char * line = NULL;
   	 		size_t length = 0;
    		ssize_t read;

    		read = getline(&line, &length, filereader);
    		//printf("%s\n", line );

    		sendto(s, (const char *) line, strlen(line),0,
			(const struct sockaddr *) &cliaddr, sizeof(cliaddr));

    		//printf("here\n");
   	 		n = recvfrom(s, (char *)buffer, MAXLINE, 0, 
			( struct sockaddr *) &cliaddr, &len);
    	 	buffer[n] = '\0';
    	 	//printf("%s\n", buffer );
    	 	//printf("%d\n",strcmp(buffer,"END") );

   	 		while (strcmp(buffer,"END\n") != 0 && strcmp(buffer,"END") != 0 && (read = getline(&line, &length, filereader))!=-1) {
   	 			//printf("NEW LINE\n");
   	 			//printf("%s", line);
   	 			
   	 			sendto(s, (const char *) line, strlen(line),0,
				(const struct sockaddr *) &cliaddr, sizeof(cliaddr));

   	 			//printf("here\n");

   	 			n = recvfrom(s, (char *)buffer, MAXLINE, 0, 
				( struct sockaddr *) &cliaddr, &len);
    	 		buffer[n] = '\0';
    	 		//printf("%s\n",buffer );
    	 		//printf("%d\n",strcmp(buffer,"END") );   
		    }
		    fclose(filereader);
   	 	}
    }

    return 0;
}
