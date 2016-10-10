/* TCP SFTP Server
 * Emily Obaditch - eobaditc
 * Teddy Brombach - tbrombac
 * Matt Reilly - mreill11
 *
 * This is our implementation of a Simple FTP Server, written in C.
 */

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <errno.h>
#include <netdb.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <errno.h>
#include <dirent.h>

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
    char buf[BUFSIZE]; // message buffer
    char *hostaddrp; // dotted decimal host addr string
    int optval; // flag value for setsockopt
    int n, k; // n = message size, k = key size
    int i;    // counter
    short len;
    char name[BUFSIZE];

    // parse command line arguments
    if (argc != 2) {
        fprintf(stderr, "usage: %s <port> <encryption key>\n", argv[0]);
        exit(1);
    }

    port = atoi(argv[1]);

    // create the parent socket
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
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
    while(1){
        if (listen(sockfd, 5) < 0)
            error("Error on binding");

    // Wait for message, send response
        clientlen = sizeof(clientaddr);
    
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

        while(1) {
            // receive a datagram from a client
            bzero(buf, BUFSIZE);
            n = read(sockfd2, buf, BUFSIZE);

            if(n < 0)
               error("Error reading from socket");
            printf("server recieved %d bytes: %s\n" , n, buf);

            /* REQUEST HANDLING BLOCK */
            if (strcmp(buf, "REQ") == 0) {
                bzero(buf, BUFSIZE);
                n = read(sockfd2, buf, BUFSIZE);    // length of filename
                len = atoi(buf);
                bzero(buf, BUFSIZE);
                n = read(sockfd2, buf, BUFSIZE);    // filename

                int i; 
                for(i=0; i<strlen(buf); i++){
                    name[i] = buf[i]; 
                }
            //name = buf;
                //name = buf;

            } else if (strcmp(buf, "UPL") == 0) {
            
            } else if (strcmp(buf, "DEL") == 0) {

            } else if (strcmp(buf, "LIS") == 0) {
                FILE *in;
                if(!(in = popen("ls", "r"))){
                    printf("error\n");        // debugging use only
                    //failed
                }

                while (fgets(name, BUFSIZE, in) != NULL) {      // name is used for list
                    // send list size, then list
                    len = strlen(name); 
                    //len = name.size();
                    //n = write(sockfd2, )
                }

                pclose(in);                         // close pipe
            } else if (strcmp(buf, "MKD") == 0) {

            } else if (strcmp(buf, "RMD") == 0) {

            } else if (strcmp(buf, "CHD") == 0) {
                bzero(buf, BUFSIZE);
                n = read(sockfd2, buf, BUFSIZE);    // length of directory name
                len = atoi(buf);
                bzero(buf, BUFSIZE);
                n = read(sockfd2, buf, BUFSIZE);    // directory name
                for(i=0; i<strlen(buf); i++){
                name[i] = buf[i]; 
                }
                //name = buf;

                DIR *dir = opendir(name);
                if (dir) {                          // directory exists
                    // send client 1 if chd success, otherwise -1
                    if (chdir(name) == 0) {
                        // send client 1
                    } else {
                        // send client -1
                    }
                } else if (ENOENT == errno) {       // directory does not exist
                    // send client -2
                }
            } else if (strcmp(buf, "XIT") == 0) {   // Close socket, return to waiting
                close(sockfd2);
                continue;
            } // No else needed, server will stay in wait mode
        

            printf("\nN: %d\n",n);
            printf("Recieved %s\n", buf);

            if(n < 0)
                error("Error reading from socket");
            printf("server recieved %d bytes: %s\n" , n, buf);

        }
    }
}
