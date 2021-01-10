// some headers
#include <arpa/inet.h> 
#include <errno.h> 
#include <netinet/in.h> 
#include <signal.h> 
#include <stdio.h> 
#include <stdlib.h>
#include <time.h> 
#include <string.h> 
#include <sys/socket.h> 
#include <sys/select.h>
#include <sys/types.h> 
#include <unistd.h> 
#include <netdb.h>
#include <stdbool.h>
#include <fcntl.h>
#include <dirent.h>
#include <pthread.h>
#include <sys/wait.h>
#include <signal.h>
#include <math.h>
#include <errno.h>
#include<sys/time.h>

int dropMessage(float p);

void *threadX(void* param);

int r_socket(int domain, int type, int protocol);

int r_bind(int socket, const struct sockaddr *address, socklen_t address_len);

ssize_t r_sendto(int sockfd, const void *buf, size_t len, int flags, const struct sockaddr *dest_addr, socklen_t addrlen);

ssize_t r_recvfrom(int sockfd, char *buf, size_t len, int flags, struct sockaddr *src_addr, socklen_t *addrlen);

int r_close(int fd);

#define INTERVAL 2 
#define TIMEOUT 2
#define BUF_SIZE 100
#define MSG_SIZE 100
#define SOCK_MRP 153
#define TABLE_SIZE 100
#define DROP_PROBALITY 0.50

#define ACK 0
#define APP 1