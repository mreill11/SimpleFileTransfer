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
        printf("Command options are REQ, UPL, DEL, LIS, MKD, RMD, CHD, and XIT\n");
        printf("Enter Command: "); 
        scanf("%s", buf);  
        n = write(sockfd,buf, strlen(buf)); 
        if(n<0)
            error("ERROR writing to socket");

        if(strcmp(buf, "LIS") != 0 && strcmp(buf,"XIT") !=0){ //if command is not LIS (strcmp returns 0 if strings are equal)
            printf("Enter filename: \n"); 
            scanf("%s", name);
            n = write(sockfd, name, sizeof(name));  
            //printf("Enter filename length: \n"); 
            // scanf("%s", len); 
            // n = write(sockfd, len, sizeof(len)); 
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
        }/* 
        printf("Looking for correct strcmp %s\n",buf);
        if (strcmp(buf, "UPL") == 0){
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
                        n = write(sockfd, currBuf, BUFSIZE);  
                        round_num = round_num + 4096;
                        bzero(currBuf, BUFSIZE); 
                    }
                }
                //n = write(sockfd, buf, BUFSIZE); 
                mhash(td, &fileBuf, 1); 
                serverHash = mhash_end(td);
                bzero(buf, BUFSIZE); 
                sprintf(buf, "%s", serverHash); 
                //printf("%s", serverHash); 
                n = write(sockfd, buf, BUFSIZE);// strlen(serverHash)); 
                if(n<0) error("error sending!"); 
                bzero(buf, BUFSIZE); 
                n = read(sockfd, buf, BUFSIZE); 
                if(n<0) error("error reading!"); 
                printf("%s\n", buf);
                bzero(buf, BUFSIZE); 
            }
        }else if (strcmp(buf, "DEL")==0){
            //receive confirmation
            bzero(buf, BUFSIZE); 
            n = read(sockfd, buf, BUFSIZE); 
            if (strcmp(buf, "1") == 0){
                bzero(buf, BUFSIZE); 
                printf("confirm deletion: Yes or No\n"); 
                scanf("%s", buf); 
                printf("%s\n", buf); 
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

        }else if(strcmp(buf, "REQ") == 0){
            printf("In REQ commande =%s\n",buf);
            struct timeval start_t, end_t;               
             //get start time
            if(gettimeofday(&start_t, NULL)==-1){
                error("Timing Error!\n"); 
                exit(1); 
            }     

            int filelen; 
            FILE *fp; 
            char finalBuf[BUFSIZE];  
            int rounds; 
            bzero(buf, BUFSIZE);

            td = mhash_init(MHASH_MD5); 
            if(td == MHASH_FAILED) return 1; 

            //SEND ACK TO CLIENT
            strcat(buf, "ready"); 

            n = write(sockfd, buf, BUFSIZE); 
            if (n < 0){
                error("Error in reading socket3\n"); 
            }
            bzero(buf, BUFSIZE); 

            //Read filesize
            n = read(sockfd, buf, BUFSIZE); 
            filelen = atoi(buf);
            if(filelen<0){
                printf("Im sorry that file does not exist!\n");
                continue;
            }
            bzero(buf, BUFSIZE); 
            //Loop to read in 4096 bit chunks
            rounds = (filelen + 4095) / 4096; 
            int round_num = 0;

            for(i=0; i<rounds; i++){
                n = read(sockfd, buf, BUFSIZE); 
                if(n<0){
                    error("error writing"); 
                } 
                fp=fopen(name, "a"); 
                fprintf(fp, buf); 
                fclose(fp); 
                strcat(finalBuf, buf); 
                bzero(buf, BUFSIZE); 
            }
            if(gettimeofday(&end_t, NULL)==-1){
                error("Timing Error!\n"); 
                exit(1);
            }
            double throughput = (end_t.tv_usec - start_t.tv_usec); 

            n = read(sockfd, buf, BUFSIZE);  
            mhash(td, &finalBuf, 1); 
            serverHash = mhash_end(td); 

            bzero(buf, BUFSIZE); 
            sprintf(buf, "Transfer was successful, throughput: %f microseconds", throughput); 
            n = write(sockfd, buf, BUFSIZE); // BUFSIZE 
           if (n<0) error("Error writing\n");
        } else if (strcmp(buf, "MKD") == 0) {
            bzero(buf, BUFSIZE);
            n = read(sockfd, buf, BUFSIZE);
            if (strcmp(buf, "1") == 0) {
                // Success!
                printf("The directory was successfully made.\n");
            } else if (strcmp(buf, "-2") == 0) {
                // Already exists
                printf("The directory already exists on server.\n");
            } else {
                printf("Error making directory.\n");
            }
        } else if (strcmp(buf, "RMD") == 0) {
            bzero(buf, BUFSIZE);
            n = read(sockfd, buf, BUFSIZE);
            if (strcmp(buf, "-1") == 0) {
                // not a directory
                printf("This directory does not exist on the server.\n");
            } else if (strcmp(buf, "1") == 0) {
                bzero(buf, BUFSIZE);
                printf("Are you sure you want to delete this directory? (yes/no) ");
                scanf("%s", buf);
                if (strcasecmp(buf, "no") == 0) {
                    printf("Delete abandoned by user!\n");
                    n = write(sockfd, buf, BUFSIZE);
                } else {
                    n = write(sockfd, buf, BUFSIZE);
                    bzero(buf, BUFSIZE);
                    n = read(sockfd, buf, BUFSIZE);
                    if (strcmp(buf, "1") == 0) {
                        printf("Directory deleted.\n");
                    } else {
                        printf("Failed to delete directory.\n");
                    }
                }
            }
        } else if (strcmp(buf, "CHD") == 0) {
            bzero(buf, BUFSIZE);
            n = read(sockfd, buf, BUFSIZE);
            if (strcmp(buf, "-2") == 0) {
                printf("The directory does not exist on the server.\n");
            } else if (strcmp(buf, "-1") == 0) {
                printf("Error changing directory.\n");
            } else {
                printf("Changed current directory.\n");
            }
        }//end elseif

         // Receive the server's reply
         //  bzero(buf, BUFSIZE); 
         //  n = read(sockfd, buf, BUFSIZE); 
         //  if (n < 0){
         //  error("ERROR reading to socket"); 
         //  }*/
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
