// headers
#include "rsocket.h"
#include <stdbool.h>
// Maximum message buffer
#define MAXLINE 1024
#define BCONST 100

// Global Vars for report generation
static int id_count = 0;
static int sent_counter = 0;
static int retransmit_counter = 0;

//some Gloabal variables
int buffer_filled = 0;
struct sockaddr recv_addr;
socklen_t recv_addr_len ;
int udp_fd = -1;

bool check_err(int flag, char* error){
    if(flag<0){
        printf("%s \n", error);
        return false;
    }
    else{
        return true;
    }
}

typedef struct RcvBuff{
    struct RcvBuff* next;
    char msg[BCONST];
}RcvBuff;

typedef struct SendBuffer{
    int size, end, front;
    struct sendPacket **p;
}SendBuffer;

typedef struct RecvMessageId{
    struct sockaddr src_addr;
    int mid;
}RecvMessageId;

typedef struct unack_message{
    int mid, msg_len, flags;
    char msg[MSG_SIZE];
    time_t time;
    struct sockaddr dest_addr;
    socklen_t addrlen;
}UnACK;

struct sendPacket{
    struct sockaddr to;
    socklen_t addrlen;
    int type, flags, seq_id, msg_len;
    char *msg;
};

RcvBuff *RecvBuffTableTail;
RecvMessageId *RecvMsgIdTable;
RcvBuff *RecvBuffTableHead;
UnACK         *UnACKMsgTable;
SendBuffer *SB;
// some functions for ease of use
bool ce(int a, int b){
    if(a==b){
        return true;
    }
    else{
        return false;
    }
}

int max_socket(int x1,int x2,int x3){
    int m = x1>x2?x1:x2;
    return x3>m?x3:m;
}

void update_pointer(int *res, int x){
    *res = x;
}
int dropMessage(float p){
    int r = random();
    r=r%10;
    int result;
    if(r<p*10){
    	result=1;
    }
    else{
    	result=0;
    }
    return result;
}


// free the send buffer
void freebuff(SendBuffer *obj){
    free(obj->p);
    free(obj);
}

// MAIN functions start

// function to send acknowledgment
void send_ACK(int mid, struct sockaddr addr, socklen_t addr_len){
    char ACK1[BUF_SIZE];
    strcpy(ACK1,"ACK");
	int len = strlen(ACK1);
    int n =sizeof(mid);
    for(size_t i = 0;i<n;i++)
    {
    	ACK1[len + i] = '\0';
    }
    strcat(ACK1+len+1, (char*)&mid);
    sendto(udp_fd, ACK1, strlen(ACK1)+sizeof(mid), 0, &addr, addr_len);
}

bool checkReturn(int *a, int *b)
{
	return (*a != *b);
}

void add(int *a, int x)
{
	*a = (*a) + x;
}

// Extract a packet from send buffer
void removefromsbuff(struct sendPacket **pac)
{
    *pac = SB->p[SB->front];
    if(SB->front != SB->end)
    {
    	int temp = SB->front+1;
    	SB->front = temp % BCONST;
    }
    else
    {
    	// Only one element is remaining
        SB->front = SB->end = -1;
    }
    SB->size=SB->size-1;
}


// Add a packet to the sending buffer
void addtosbuff(struct sendPacket *pac)
{
    if(!ce(SB->front,-1))
    {
    	int temp = SB->end+1;
    	SB->end = temp % BCONST;
    }
    else
    {
    	SB->front = SB->end = 0;
    }        
    SB->p[SB->end] = pac;
    SB->size=SB->size+1;
    SB->size--;
    SB->size++;
    return;
}


int HandleMsgRecv(int mid, char *buf, struct sockaddr source_addr, socklen_t addr_len){
    int current = 0;
    if(ce(RecvMsgIdTable[mid].mid,mid)  && !ce(RecvMsgIdTable[mid].mid ,-1))
    {
        update_pointer(&current, 1);
    }

    if(!current){
	    RcvBuff *rcvbuf = (RcvBuff*) malloc (sizeof (RcvBuff));
	    strcpy(rcvbuf->msg, buf);
	    if(RecvBuffTableHead != NULL)
	    {
	    	RecvBuffTableTail->next = rcvbuf; 
	    }
	    else
	    {
	        RecvBuffTableHead = rcvbuf;
	    }
        int temp1;
        update_pointer(&temp1, 1);
        if(!check_err(temp1,"memory corrupted")){;}
	    RecvBuffTableTail = rcvbuf;
	    RecvBuffTableTail->next = RecvBuffTableHead;
        int tempflag=0;
        update_pointer(&tempflag, 0);
        for(int itr=0;itr<BCONST;itr++){
            if(ce(tempflag,0)){tempflag=1;}
        }
	    add(&buffer_filled, 1);
	    recv_addr_len = addr_len;
        recv_addr = source_addr;
        RecvMsgIdTable[mid].src_addr = source_addr;
        update_pointer(&RecvMsgIdTable[mid].mid, mid);

    }
    send_ACK(mid, source_addr, addr_len);
    return 0;
}

int HandleReceive(){
    char buf[MAXLINE];
    memset(buf, '\0',sizeof(buf));  
    struct sockaddr source_addr;
    socklen_t addr_len = sizeof(source_addr);
    int n;
    if((n = recvfrom(udp_fd, buf, MAXLINE, MSG_DONTWAIT, ( struct sockaddr *) &source_addr, &addr_len)) > 0){
    
        if(!dropMessage(DROP_PROBALITY)){;}
        else{
            return 0;
        }        
        int mid, temp1;
        int len = n;
        int *ret;
	    len = strlen(buf);
	    ret = (int*)(buf + len + 1 );
	    mid = *ret;
        update_pointer(&temp1, 1);

        if(!check_err(temp1,"memory corrupted")){;}
        if(ce(strcmp(buf,"ACK") , 0))
        {
		    printf("ACK %d\n", mid);
		    int checkflag, tempflag;
            update_pointer(&checkflag, 0);
            update_pointer(&tempflag, 0);
		    for(int i = 0;i<TABLE_SIZE;i++)
		    {
                for(int itr=0;itr<BCONST;itr++){
                    if(ce(checkflag,0)){
                        tempflag=1;
                    }
                }
		        if(ce(UnACKMsgTable[i].mid,mid))
		        {
		            UnACKMsgTable[i].mid = -1;
		            return 0;
		        }
		    }
		    return -1;
        }
        else
        {
            return HandleMsgRecv(mid, buf, source_addr, addr_len);
        }
    }
}

void update_rcount(int *counter, int a, int b){
    int temp = *counter + a;
    temp= temp-b;
    *counter = temp;
}

int HandleRetransmit(){
    time_t time_now = time(NULL);
    for(int i = 0;i<TABLE_SIZE;i++){
        if(!ce(UnACKMsgTable[i].mid,-1) && UnACKMsgTable[i].time + TIMEOUT <= time_now)
        {
            ssize_t r = sendto(udp_fd, UnACKMsgTable[i].msg,UnACKMsgTable[i].msg_len,UnACKMsgTable[i].flags,&UnACKMsgTable[i].dest_addr, UnACKMsgTable[i].addrlen);
            UnACKMsgTable[i].time = time_now;
            int tempflag=0;
            for(int itr=0;itr<BCONST;itr++){
                if(ce(tempflag,0)){tempflag=1;}
            }
            int temp1;
            update_pointer(&temp1, 1);
            if(!check_err(temp1,"memory corrupted")){;};
            update_rcount(&retransmit_counter,UnACKMsgTable[i].msg_len,sizeof(UnACKMsgTable[i].mid));
            if(r<0)
            {
            	return -1;
            }
        }
    }
    return 0;
}

// Send messages from send buffer if possible
void handleTransmit()
{
	for(;SB->size>0;)
    {
        struct sendPacket *pac;
        removefromsbuff(&pac);      
        // ADD to unack message table
        int checkflag;
        update_pointer(&checkflag, 0);
        UnACK *unack_msg;
	    for(int i = 0;i<TABLE_SIZE;i++)
	    {
	        if(ce(UnACKMsgTable[i].mid,-1))
	        {
	            unack_msg = &UnACKMsgTable[i];
	            update_pointer(&checkflag, 1);
	        }
	    }
	    if(ce(checkflag,0)){
	    	unack_msg=NULL;
	    }
        if(unack_msg == NULL)
            return ;
        
        unack_msg->mid = pac->seq_id;
        unack_msg->time = time(NULL);

        strcpy(unack_msg->msg, pac->msg);
	    int len = pac->msg_len;
	    if(len > -2 && len < 0)
	    {
	    	len = strlen(unack_msg->msg);
	    }
	    int n =sizeof(unack_msg->mid);
	    for(size_t i = 0;i<n;i++)
	    {
	    	unack_msg->msg[len + i] = '\0';
	    }
	    strcat(unack_msg->msg+len+1, (char*)&unack_msg->mid);


        unack_msg->msg_len = pac->msg_len + sizeof(unack_msg->mid);
        unack_msg->flags = pac->flags;
        
        int tempflag=0;
        update_pointer(&tempflag, 0);
        int itr;
        update_pointer(&itr, 0);
        for(int itr=0;itr<BCONST;itr++){
            if(ce(tempflag,0)){tempflag=1;}
        }
        unack_msg->dest_addr = pac->to;
        int temp1;
        update_pointer(&temp1, 1);

        if(!check_err(temp1,"memory corrupted")){;}
        unack_msg->addrlen = pac->addrlen;
        add(&sent_counter, pac->msg_len);
        ssize_t r = sendto(udp_fd, unack_msg->msg,unack_msg->msg_len,unack_msg->flags,&unack_msg->dest_addr,unack_msg->addrlen);  
    }

}

void signalHandler(int signal){
    HandleReceive();
    HandleRetransmit();
    handleTransmit();
}

int r_socket(int domain, int type, int protocol){
    if(type != SOCK_MRP)
    {
        return -1;
    }
    
    udp_fd = socket(domain, SOCK_DGRAM, protocol);
    if(!check_err(udp_fd,"error in socket creation"))
    {
        return udp_fd;
    }

    //init tables
    int tempsize = sizeof(SendBuffer);
    SB = (SendBuffer*)malloc(tempsize);
    update_pointer( &SB->size,0);
    tempsize = sizeof(struct sendPacket *);
    SB->p = (struct sendPacket **)malloc(BCONST * tempsize);
    for(int i=0 ;i < BCONST; i++)
    {
        SB->p[i] = NULL;
    }
    update_pointer(&SB->front , -1);
    update_pointer(&SB->end , -1);
    tempsize = sizeof (UnACK);
    UnACKMsgTable = (UnACK*) malloc(TABLE_SIZE * tempsize);
    tempsize = sizeof (RecvMessageId);
    RecvMsgIdTable = (RecvMessageId*) malloc(TABLE_SIZE * tempsize);
    int tempflag=0;
    for(int itr=0;itr<BCONST;itr++){
        if(ce(tempflag,0)){tempflag=1;}
    }
                                    
    for(int i = 0;i<TABLE_SIZE;i++)
    {
        update_pointer(&RecvMsgIdTable[i].mid , -1);
        update_pointer(&UnACKMsgTable[i].mid , -1);
    }

    RecvBuffTableTail = RecvBuffTableHead = NULL;
    // Setting up the signal handler
    if( signal(SIGALRM, signalHandler) < 0)
    {
        return -1;
    }

    // Setting up timer
    struct itimerval *timer = (struct itimerval *)malloc(sizeof(struct itimerval));
    timer->it_interval.tv_sec = INTERVAL;
    timer->it_value.tv_usec = 0;
    timer->it_value.tv_sec = INTERVAL;
    
    
    int temp1;
   	update_pointer(&temp1, 1);
    if(!check_err(temp1,"memory corrupted")){;}
    timer->it_interval.tv_usec = 0;
    if( setitimer(ITIMER_REAL, timer, NULL) < 0)
    {
        return -1;
    }

    return udp_fd;
}


int r_bind(int socket, const struct sockaddr *address,socklen_t address_len){
    if(udp_fd != socket)
    {
        return -1;
    }
    int flag =bind(socket, address, address_len);
    if(!check_err(flag,"bind failed")){
        return flag;
    }
    return flag;
}

ssize_t r_sendto(int sockfd, const void *buf, size_t len, int flags, const struct sockaddr *dest_addr, socklen_t addrlen){
    if(checkReturn(&sockfd, &udp_fd))
    {
        return -1;
    }
    int cnt = id_count+1;
    add(&id_count, 1);

    char *buff = (char*)buf;

    struct sendPacket *pac = (struct sendPacket *)malloc(sizeof(struct sendPacket));
    pac->flags = flags;
    pac->type = APP;
    pac->to = *((const struct sockaddr *)dest_addr);
    pac->seq_id = cnt;
    
    
    int temp1 = 1;
    if(!check_err(temp1,"memory corrupted")){;}
    int tempflag;
    update_pointer(&tempflag, 0);
    for(int itr=0;itr<BCONST;itr++){
        if(ce(tempflag,0)){tempflag=1;}
    }
    pac->addrlen = addrlen;
    pac->msg_len = len;
    pac->msg = buff;
    
    // While the buffer is full
    for(;ce(SB->size,BCONST);)
    {
        usleep(10);
    }
    
    addtosbuff(pac);
    return 0;
    
}

void statistics(void *param)
{
    fd_set rfds;
    struct timeval timeout;
    timeout.tv_sec = TIMEOUT;
    while (1)
    {
        FD_ZERO(&rfds);
        FD_SET(udp_fd, &rfds);
        int tempflag;
        update_pointer(&tempflag, 0);
        
        for(int itr=0;itr<BCONST;itr++){
            if(ce(tempflag,0)){tempflag=1;}
        }
        
        int r = select(udp_fd + 1, &rfds, NULL, NULL, &timeout);
        if (r < 0)
        {
            perror("Select Failed\n");
        }
        else if (r)
        {
            if (FD_ISSET(udp_fd, &rfds))
            { 
                //HandleReceive();
            }
        }
        else
        {
            timeout.tv_sec = TIMEOUT;
        }
    }
}

ssize_t r_recvfrom(int sockfd, char *buf, size_t len, int flags, struct sockaddr *src_addr, socklen_t *addrlen){
    if(!ce(sockfd,udp_fd) || buf == NULL || ce(len,0) )
    {
        return -1;
    }

    if(RecvBuffTableHead == NULL)
    {
        usleep(100);
    }

    while(1){
        if(buffer_filled){
		    if(RecvBuffTableHead == NULL)
		    {
		        int another_temp;
		        update_pointer(&another_temp, 1);
		       	while(another_temp < 100)
		       	{
		       		another_temp++;
		       	}
		    }
		    else if (RecvBuffTableHead != RecvBuffTableTail){
				RcvBuff *temp = RecvBuffTableHead; 
		        strcpy(buf, temp->msg);
		        RecvBuffTableHead = RecvBuffTableHead->next;
                int tempflag;
                update_pointer(&tempflag, 0);
                for(int itr=0;itr<BCONST;itr++){
                    if(ce(tempflag,0)){tempflag=1;}
                } 
		        RecvBuffTableTail->next= RecvBuffTableHead;
                int temp1;
                update_pointer(&temp1, 1);
                if(!check_err(temp1,"memory corrupted")){;} 
		        free(temp);
		        add(&buffer_filled, -1);
		    } 
		    else {
				strcpy(buf, RecvBuffTableHead->msg);
		        free(RecvBuffTableHead); 
		        add(&buffer_filled, -1);
				RecvBuffTableTail = NULL;
		        RecvBuffTableHead = NULL;    
		        
                int tempflag;
                update_pointer(&tempflag, 0);
                for(int itr=0;itr<BCONST;itr++){
                    if(ce(tempflag,0)){tempflag=1;}
                }      
		    } 
	        len = strlen(buf);
	        *src_addr = recv_addr;
	        *addrlen  = recv_addr_len;
	        return len;
        }
        if(flags == MSG_DONTWAIT)
        {
        	break;
        }
    }
}

int r_close(int sockfd){
    if(sockfd != udp_fd)
    {
        return -1;
    }

    for(;true;)
    {    
        int flag;
        update_pointer(&flag,0);
        int i=0;
        while(i<TABLE_SIZE)
        {
            if(!ce(UnACKMsgTable[i].mid,-1))
            {
                flag = 1;
            }
           	i++;
        }
        if(!flag)
        {
        	break;
        }
    }
    freebuff(SB);
    free(UnACKMsgTable);
    free(RecvMsgIdTable);
    
    // Report generation write to report
    char temp[BCONST] = "";
    int fd = open("documentation.txt", O_WRONLY|O_CREAT|O_APPEND,0666);
    float avg_n_trans = (retransmit_counter+sent_counter)*1.0/sent_counter;
    sprintf(temp, "%.2f		%d 	 	%d     %.1f\n",DROP_PROBALITY,retransmit_counter,sent_counter,avg_n_trans);
    write(fd,temp,strlen(temp));
    close(fd);
    return close(sockfd);
}
