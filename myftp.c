/* TCP SFTP Client
 * Emily Obaditch - eobaditc
 * Teddy Brombach - tbrombac
 * Matt Reilly - mreill11
 *
 * This is our implementation of a Simple FTP Client, written in C.
 */

#include <stdio.h>
#include <stdlib.h>
#include <mhash.h>
#include <limits.h>
#include <sys/stat.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/stat.h>
#include <sys/time.h>

 
#define BUFSIZE 4096
 
void readFile(char *dest, char *fname);
void error(char *msg);
 
int main(int argc, char *argv[]) {
    int XIT = 0;
    MHASH td; 
    char filecontent[BUFSIZE]; 
    struct sockaddr_in serveraddr;
    struct hostent *server;
    int sockfd, filesize, portno, n, k, serverlen;
    char *hostname;
    char *command;
    unsigned char * serverHash; 
    char name[BUFSIZE]; 
    char len [BUFSIZE]; 
    char buf[BUFSIZE];
    int len_s; 
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
            printf("Enter filename: \n"); 
            scanf("%s", name);
            n = write(sockfd, name, sizeof(name));  
            printf("Enter filename length: \n"); 
            scanf("%s", len); 
            n = write(sockfd, len, sizeof(len)); 
            printf("IN HERE\n");
        }
        // Send the message to the server
        if(strcmp(buf,"XIT")==0){
            printf("The connection has been closed\n");
            exit(0);

        } else if(strcmp(buf, "LIS") == 0){
            n = write (sockfd, buf, sizeof(buf)); 
            if ( n<0) {
                error("Error writing to socket"); 
            }
            //bzero(buf, BUFSIZE); 
            n = read(sockfd, buf, sizeof(buf)); 
            printf("%s\n",buf);
            if (n < 0){
                error("Error writing to socket"); 
            }
        } if (strcmp(buf, "UPL") == 0){
            struct stat st;
            int rounds, j, size;
            int j_limit = 4095; 
            char currBuf[BUFSIZE]; 
            
            td = mhash_init(MHASH_MD5); 
            if(td == MHASH_FAILED) return 1; 

            bzero(buf, BUFSIZE); 
            //receive ACK
            n = read(sockfd, buf, BUFSIZE); 
            if (strcmp(buf, "ready") == 0){
                //Check for access
                if(access(name, F_OK) == -1){
                    printf("File does not exist\n"); 
                    break; 
                }
                stat(name, &st); 
                size = st.st_size;
                bzero(buf, BUFSIZE); 
                sprintf(buf, "%d", size);  
                //send file size
                n = write(sockfd, buf, BUFSIZE); 
                if (n<0){
                    error("write error"); 
                }
                char fileBuf[size+1];
                //read file into buffer to be sent 
                readFile(fileBuf, name); 
                if(size < 4096){
                    n = write(sockfd, fileBuf, BUFSIZE); 
                    if(n < 0 ) error("Error sending\n"); 
                }
                else{
                    rounds = (size + 4095) / 4096; 
                    int round_num = 0; 
                    for(i = 0; i<rounds; i++){
                        for(j = 0; j<4095; j++){
                            currBuf[j] = fileBuf[round_num+j]; 
                        }
                        //currBuf[4095] = '\0'; 
                        n = write(sockfd, currBuf, strlen(currBuf)); //BUFSIZE); 
                        round_num = round_num + 4096;
                        bzero(currBuf, BUFSIZE); 
                    }
                }
                mhash(td, &fileBuf, 1); 
                serverHash = mhash_end(td); 
                n = write(sockfd, serverHash, sizeof(serverHash)); 
            }
        }else if (strcmp(buf, "DEL")==0){
            //receive confirmation
            bzero(buf, BUFSIZE); 
            n = read(sockfd, buf, BUFSIZE); 
            if (strcmp(buf, "1") == 0){
                bzero(buf, BUFSIZE); 
                printf("confirm deletion: Yes or No\n"); 
                scanf("%s", buf); 
                n = write(sockfd, buf, BUFSIZE); 
                if (n < 0) error("Error writing to socket"); 
                if (strcmp(buf, "Yes") == 0){
                    //wait for confirmation
                    bzero(buf, BUFSIZE); 
                    n = read(sockfd, buf, BUFSIZE); 
                    if (n < 0) error("error reading");
                    printf("Recieved delete confirmation: %s\n", buf); 
                } else{
                    printf("Delete abandoned by User!\n"); 
                }
            }
            else{
                printf("File does not exist\n"); 
            }

        }//end elseif
        
       /* // Receive the server's reply
        bzero(buf, BUFSIZE); 
        n = read(sockfd, buf, BUFSIZE); 
        if (n < 0){
            error("ERROR reading to socket"); 
        }*/
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
