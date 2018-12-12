/*
** client.c -- a stream socket client demo
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <time.h>

#include <arpa/inet.h>

#define MAXDATASIZE (102400) // max number of bytes we can get at once 
#define RECVBUFFER (2097152) // receive buffer for this socket

int main(int argc, char *argv[])
{
    int sockfd, numbytes;  
    char buf[MAXDATASIZE];
    struct addrinfo hints, *servinfo, *p;
    int rv;

    //receiver usage
    if (argc != 3) {
        printf("usage:\n%s hostname port\n", argv[0]);
        exit(1);
    }


    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    if ((rv = getaddrinfo(argv[1], argv[2], &hints, &servinfo)) != 0) {
        printf("getaddrinfo: %s\n", gai_strerror(rv));
        return 1;
    }
    // loop through all the results and connect to the first we can
    for(p = servinfo; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype,
                p->ai_protocol)) == -1) {
            perror("client: socket");
            continue;
        }

        if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockfd);
            perror("client: connect");
            continue;
        }

        break;
    }
    if (p == NULL) {
        printf("client: failed to connect\n");
        return 2;
    }

    printf("connected.");
    freeaddrinfo(servinfo); // all done with this structure
 
    //to keep track of each second data rate
    time_t current_second = time(NULL);
    time_t last_second = time(NULL);
    double total_bytes_recvd = 0;
    int count = 0;

    //set the receiving buffer
    int recvBuff = RECVBUFFER;
    if (setsockopt(sockfd, SOL_SOCKET, SO_RCVBUF, &recvBuff, sizeof(recvBuff)) < 0) {
        printf("CAN'T SET SOCKET RECV BUFF");
        exit(1);
    }

    while(1) {
        numbytes = recv(sockfd, buf, MAXDATASIZE, MSG_DONTWAIT);
    	if(numbytes == -1) {
             numbytes = 0;
        }
        current_second = time(NULL);
        if(current_second != last_second) {
            printf("Total Bytes: %lf Mbps in second - %i\n", (total_bytes_recvd/(1000*1000))*8, count);
            count++;
            total_bytes_recvd = 0;
            last_second = current_second;
        }
        total_bytes_recvd += numbytes;
    }

    close(sockfd);
    return 0;
}
