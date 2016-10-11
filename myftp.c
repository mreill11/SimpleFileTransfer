/* TCP SFTP Client
 * Emily Obaditch - eobaditc
 * Teddy Brombach - tbrombac
 * Matt Reilly - mreill11
 *
 * This is our implementation of a Simple FTP Client, written in C.
 */

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
    int XIT = 0;
    struct sockaddr_in serveraddr;
    struct hostent *server;
    struct timeval start, end;
    int sockfd, portno, n, k, serverlen;
    char *hostname;
    char *command;
    //short int len; 
    //char name[BUFSIZE]; 
    char buf[BUFSIZE];
    char key[BUFSIZE];
    int i=0;
 
    /* check command line arguments */
    // CHANGE <text or file name>
    if (argc != 3) {
        fprintf(stderr,"usage: %s <hostname> <port>\n", argv[0]);
        exit(0);
    }
    hostname = argv[1];
    portno = atoi(argv[2]);
 
 
    // Load buffer, 
  
    bzero(buf, BUFSIZE);
    short len; 
    char name[BUFSIZE]; 
    //if (strcmp(buf, "XIT") == 0){
    //    printf("XIT\n");
    //    XIT = 1; 
    //}
    //if(strcmp(buf, "LIS") == 0){
    //    printf("Enter length of filename and file name: "); 
    //    scanf("%hd %s", len, name); 
    //}

    // Create the socket 
    sockfd = socket(AF_INET, SOCK_STREAM, 0);

    if (sockfd < 0){
        error("ERROR opening socket");
    }
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

    /* connect */
    if (connect(sockfd,(struct sockaddr*) &serveraddr, sizeof(serveraddr)) < 0){
        error ("ERROR connecting"); 
    }


    while(1){
        printf("Enter Command: "); 
        scanf("%s", buf);  
        n = write(sockfd,buf, strlen(buf)); 
        if(n<0)
            error("ERROR writing to socket");

        if(strcmp(buf, "LIS") != 0 && strcmp(buf,"XIT") !=0){ //if command is not LIS (strcmp returns 0 if strings are equal)
            printf("Enter length of filename and the file name: ");
            scanf("%s %s", &len, name);
            printf("IN HERE\n");
        }


        // Send the message to the server


        if(strcmp(buf,"XIT")==0){
            printf("The connection has been closed\n");
            exit(0);
        } else if(strcmp(buf, "LIS") != 0){
            printf("STRCMP\n");
            n = write (sockfd, (const char *)&len, sizeof(len)); 
            printf("WRITE\n");
            if ( n<0) {
                error("Error writing to socket"); 
            }
            n = write (sockfd, name, strlen(name)); 
            printf("write 2 %d\n",n);
            if (n < 0){
                error("Error writing to socket"); 
            }
        }
        
        // Receive the server's reply
        bzero(buf, BUFSIZE); 
        n = read(sockfd, buf, BUFSIZE); 
        if (n < 0){
            error("ERROR reading to socket"); 
        }
        bzero(buf, BUFSIZE); 

    }
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
