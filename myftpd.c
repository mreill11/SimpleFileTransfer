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

void readFile(char *dest, char*fname);

// error handling
void error(char *msg) {
  perror(msg);
  exit(1);
}

int main(int argc, char *argv[]) {
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
    char *name;
    char *len_string; 
    char com[BUFSIZE];
    char* path;
    char * choice; 
    char* filename;
    //strcat(filename, "path.txt");
    
    //system("pwd > path.txt");
   // readFile(path, (char *)"path.txt");
   // printf("Path = %s",path);

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
            printf("Before read 1\n");
            n = read(sockfd2, buf, BUFSIZE);
            strcpy(com,buf);
            printf("Command = %s\n",com);

            if(n < 0)
               error("Error reading from socket");
            printf("server recieved %d bytes: %s\n" , n, buf);

            /* REQUEST HANDLING BLOCK */
            if(strcmp(com,"XIT") == 0){
                close(sockfd2);
                break;
            }else if(strcmp(com,"LIS") == 0){
                char choice[20] =  "ls > lsOutput.txt";
                system(choice); 
                bzero(buf, BUFSIZE); 
                printf("in LIS\n"); 
                readFile(buf, "lsOutput.txt");
                printf("%s\n", buf); 
                n = write(sockfd2, buf, BUFSIZE);
                if (n <0) error("Error in LIS\n"); 
            }else{
            bzero(buf,BUFSIZE);
            printf("before read 2\n");
            n = read(sockfd2, buf, BUFSIZE);
            printf("%s", buf); 
            name =  buf; 
            bzero(buf, BUFSIZE); 
            n = read(sockfd2, buf, BUFSIZE); 
            len_string = buf;
            len = atoi(len_string); 
            printf("%s%s", name, len_string); 
            }
            if (strcmp(com, "REQ") == 0) {

            } else if (strcmp(com, "UPL") == 0) {
                int filelen; 
                FILE *fp; 
                char * currBuf;  
                int rounds; 
                bzero(buf, BUFSIZE); 
                //SEND ACK TO CLIENT
                strcat(buf, "ready"); 
                printf("%s\n", buf); 
                n = write(sockfd2, buf, BUFSIZE); 
                if (n < 0){
                    error("Error in reading scoket3\n"); 
                }
                bzero(buf, BUFSIZE); 
                //read filesize
                n = read(sockfd2, buf, BUFSIZE); 
                filelen = atoi(buf);  
                bzero(buf, BUFSIZE); 
                rounds = (filelen + 4095) / 4096; 
                int round_num = 0;
                printf("HERE"); 
                for(i=0; i<rounds; i++){
                    //read 4096 bytes of file
                    n = read(sockfd2, buf, BUFSIZE); 
                    if(n<0){
                        error("error writing"); 
                    } 
                    fp=fopen(name, "a"); 
                    fprintf(fp, buf); 
                    fclose(fp); 
                    bzero(buf, BUFSIZE); 
                }               

            
            } else if (strcmp(com, "DEL") == 0) {
                printf("HERE\n"); 
                if(access(name, F_OK) != -1){
                    strcat(buf, "1"); 
                    n = write(sockfd2, buf, BUFSIZE); 
                    if (n<0) error("error sending"); 
                    printf("%s", buf); 
                    bzero(buf, BUFSIZE); 
                    n = read(sockfd2, buf, BUFSIZE); 
                    if (strcmp(buf, "Yes") == 0){
                        sprintf(choice, "rm %s", name); 
                        system(choice); 
                        n = write(sockfd2, buf, BUFSIZE); 
                        if (n<0) error("error writing"); 
                    }
   
                } else{
                    strcat(buf, "-1"); 
                }                
            } else if (strcmp(com, "MKD") == 0) {
                system("cd ..");
                system("ls > output");

            } else if (strcmp(com, "RMD") == 0) {

            } else if (strcmp(com, "CHD") == 0) {
                bzero(buf, BUFSIZE);
                printf("READ\n");

                n = read(sockfd2, buf, BUFSIZE);    // length of directory name
                if(n < 0)
                    error("Error reading from socket");
            
                printf("read\n");
                len = atoi(buf);
                bzero(buf, BUFSIZE);

                n = read(sockfd2, buf, BUFSIZE);    // directory name
                if(n < 0)
                    error("Error reading from socket");
            
                printf("DONE\n");
                for(i=0; i<strlen(buf); i++){
                name[i] = buf[i]; 
                }
                //name = buf;

                bzero(buf, BUFSIZE);
                DIR *dir = opendir(name);
                if (dir) {                          // directory exists
                    // send client 1 if chd success, otherwise -1
                    if (chdir(name) == 0) {
                        // send client 1
                        buf[0] = '1';
                    } else {
                        // send client -1
                        buf[0] = '-';
                        buf[1] = '1';
                    }
                } else if (ENOENT == errno) {       // directory does not exist
                    // send client -2
                    buf[0] = '-';
                    buf[1] = '2';
                }
            }
        
            n = write(sockfd2, buf, strlen(buf));

            printf("\nN: %d\n",n);
            printf("Recieved %s\n", buf);

            if(n < 0)
                error("Error reading from socket");
            printf("server recieved %d bytes: %s\n" , n, buf);

        }
    }
}

void readFile(char *dest, char *fname){
    FILE *fp = fopen(fname, "r");
    if(fp != NULL){
        size_t new_len = fread(dest, sizeof(char), BUFSIZE, fp);
        if(ferror(fp) != 0){
            fputs("error reading file" , stderr);
        } else{
            dest[new_len++] = '\0';
        }
        fclose(fp);
    }
}
