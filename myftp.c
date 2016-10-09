#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/time.h>
 
#define BUFSIZE 4096
 
void readFile(char *dest, char *fname);
void error(char *msg);
 
int main(int argc, char **argv) {
    struct sockaddr_in serveraddr;
    struct hostent *server;
    struct timeval start, end;
    int sockfd, portno, n, k, serverlen;
    char *hostname;
    char buf[BUFSIZE];
    char key[BUFSIZE];
    int i=0;
 
    /* check command line arguments */
    // CHANGE <text or file name>
    if (argc != 4) {
        fprintf(stderr,"usage: %s <hostname> <port> <text or file name>\n", argv[0]);
        exit(0);
    }
    hostname = argv[1];
    portno = atoi(argv[2]);
   
    // Create the socket
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
   
    if (sockfd < 0)
        error("ERROR opening socket");
 
    // Load DNS Entry
    server = gethostbyname(hostname);
    if (server == NULL) {
            fprintf(stderr,"ERROR, no such host as %s\n", hostname);
            exit(0);
    }
 
    // Obtain server address
    bzero((char *) &serveraddr, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;
    bcopy((char *)server->h_addr, (char *)&serveraddr.sin_addr.s_addr, server->h_length);
    serveraddr.sin_port = htons(portno);
 
    // Load buffer, check if it is a file or string
    bzero(buf, BUFSIZE);
    if (access(argv[3], F_OK) != -1) {
            readFile(buf, argv[3]);
    } else {
        strcpy(buf, argv[3]);
    }
 
    // Send the message to the server
    serverlen = sizeof(serveraddr);
    n = sendto(sockfd, buf, strlen(buf), 0, &serveraddr, serverlen);
    gettimeofday(&start, NULL);
    if (n < 0)
            error("ERROR in sendto");
   
    // Receive the server's reply
    n = recvfrom(sockfd, buf, BUFSIZE, 0, &serveraddr, &serverlen);
    gettimeofday(&end, NULL);
    k = recvfrom(sockfd, key, BUFSIZE, 0, &serveraddr, &serverlen);
 
    if (k < 0) {
        error("ERROR in key");
    } else if (n < 0)  {
        error("ERROR in recvfrom");
    }
 
    long double rtt = (end.tv_sec * (int)1e6 + end.tv_usec) - (start.tv_sec * (int)1e6 + start.tv_usec);
 
    // decrypt the key
    for (i = 0; i < n; i++) {
        buf[i] = buf[i] ^ key[i%k];
    }
 
    // print the servers reply
    printf("Echo from server: %s\n", buf);
    printf("Key: %x\n", key);
    printf("RTT: %Lf microseconds\n", rtt);
    return 0;
}
 
// Read file into buffer
void readFile(char *dest, char *fname) {
    FILE *fp = fopen(fname, "r");
    if (fp != NULL) {
        size_t new_len = fread(dest, sizeof(char), BUFSIZE, fp);
        if (ferror(fp) != 0) {
            fputs("Error reading file", stderr);
        } else {
            dest[new_len++] = '\0';
        }
        fclose(fp);
    }
}
 
// error handling
void error(char *msg) {
    perror(msg);
    exit(0);
}