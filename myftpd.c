// UDP Server
// teddy brombach
// tbrombac
// Emily Obaditch
// eobaditc
// Matt Reilly
// Mreill11

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>
#include <sys/time.h>

#define BUFSIZE 4096

// error handling
void error(char *msg) {
  perror(msg);
  exit(1);
}

int main(int argc, char **argv) {
	int sockfd; // socket 
	int sockfd2;
	int port; // port number
	int clientlen; // byte size of client's address
	struct sockaddr_in serveraddr; // server's addr
	struct sockaddr_in clientaddr; // client addr
	struct hostent *hostp; // client host info
	struct timeval timestruct;
	time_t curTime;
	char time[30];
	char timeBuf[60];
	char buf[BUFSIZE]; // message buffer
	char key[BUFSIZE]; // decryption key
	char *hostaddrp; // dotted decimal host addr string
	int optval; // flag value for setsockopt
	int n, k; // n = message size, k = key size
	int i;    // counter

	// parse command line arguments
	if (argc != 2) {
		fprintf(stderr, "usage: %s <port> <encryption key>\n", argv[0]);
		exit(1);
	}

	port = atoi(argv[1]);

	// create the parent socket
	sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	if (sockfd < 0) 
		error("ERROR opening socket");

	// setsockopt: rerun server faster
	optval = 1;
	setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, (const void *)&optval, sizeof(int));

	// build server internet address
	bzero((char *) &serveraddr, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
	serveraddr.sin_port = htons((unsigned short)port);

	// bind the parent socket and port
	if (bind(sockfd, (struct sockaddr *) &serveraddr, sizeof(serveraddr)) < 0) 
		error("ERROR on binding");

	//Listen 
	if (listen(sockfd, 5) < 0)
	    error("Error on binding");

	// Wait for message, send response
	clientlen = sizeof(clientaddr);
	while (1) {

        sockfd2 = accept( sockfd, (struct sockaddr *) &clientaddr, &clientlen);
        if(sockfd2 < 0)
            error("Error on accept");
        
        hostp = gethostbyaddr((const char *)&clientaddr.sin_addr.s_addr, sizeof(clientaddr.sin_addr.s_addr), AF_INET);

        if (hostp == NULL)
			error("ERROR on gethostbyaddr");
		hostaddrp = inet_ntoa(clientaddr.sin_addr);
		if (hostaddrp == NULL)
			error("ERROR on inet_ntoa\n");

		printf("Server connected with %s (%s)\n", hostp->h_name, hostaddrp);

		// receive a datagram from a client
		bzero(buf, BUFSIZE);
		n = read(sockfd2, buf, BUFSIZE);

		if(n < 0)
		    error("Error reading from socket");
		printf("server recieved %d bytes: %s" , n, buf);

		n = write(sockfd2, buf, strlen(buf));
		if(n<0)
		    error("Error writing to socket");

		close(sockfd2);	
	}
}
