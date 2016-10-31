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
#include <mhash.h>
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
int path_is_directory (const char* path);

// error handling
void error(char *msg) {
  perror(msg);
  exit(1);
}

int main(int argc, char *argv[]) {
    int sockfd; // socket 
    int sockfd2;
    MHASH td; 
    int port; // port number
    int clientlen; // byte size of client's address
    struct sockaddr_in serveraddr; // server's addr
    struct sockaddr_in clientaddr; // client addr
    struct timeval start_t, end_t; 
    struct hostent *hostp; // client host info
    char buf[BUFSIZE]; // message buffer
    char *hostaddrp; // dotted decimal host addr string
    int optval; // flag value for setsockopt
    int n, k; // n = message size, k = key size
    int i;    // counter
    short len;
    char *name;
    char *len_string; 
    unsigned char * serverHash; 
    char com[BUFSIZE];
    char* path;
    char choice[BUFSIZE]; 
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
            //printf("Before read 1\n");
            n = read(sockfd2, buf, BUFSIZE);
            strcpy(com,buf);
            //printf("Command = %s\n",com);

            if(n < 0)
               error("Error reading from socket");
            //printf("server recieved %d bytes: %s\n" , n, buf);

            /* REQUEST HANDLING BLOCK */
            if(strcmp(com,"XIT") == 0){
                close(sockfd2);
                break;
            }else if(strcmp(com,"LIS") == 0){
                char choice[20] =  "ls > lsOutput.txt";
                system(choice); 
                bzero(buf, BUFSIZE); 
                //printf("in LIS\n"); 
                readFile(buf, "lsOutput.txt");
                //printf("%s\n", buf); 
                n = write(sockfd2, buf, BUFSIZE);
                if (n <0) error("Error in LIS\n"); 
            }else{
            bzero(buf,BUFSIZE);
            n = read(sockfd2, name, BUFSIZE);
            //n = read(sockfd2, buf, BUFSIZE); 
            //len_string = buf;
            //len = atoi(len_string); 
            //printf("name: %s len: %s", name, len_string); 
            bzero(buf, BUFSIZE); 
            } if (strcmp(com, "REQ") == 0) {
		    int s;
			struct stat st;
	       	    	int rounds, j, size;
		    	int j_limit = 4095; 
		    	char currBuf[BUFSIZE]; 
		    
		    	td = mhash_init(MHASH_MD5); 
		    if(td == MHASH_FAILED) return 1; 

		    bzero(buf, BUFSIZE); 
		    //receive ACK
		    n = read(sockfd2, buf, BUFSIZE); 
		    if (strcmp(buf, "ready") == 0){
                //printf("In REQ! and ready\n");
		        //Check for access
		        if(access(name, F_OK) == -1){
		            s = -1;
				bzero(buf,BUFSIZE);
				sprintf(buf,"%d",s);
				n = write(sockfd2,buf,BUFSIZE);
				if(n<0) error("ERROR IN READING");
				break;
		        }
		        stat(name, &st); 
		        size = st.st_size;
		        bzero(buf, BUFSIZE); 
		        sprintf(buf, "%d", size);  
		        //send file size
		        n = write(sockfd2, buf, BUFSIZE); 
		        if (n<0){
		            error("write error"); 
		        }
		        char fileBuf[size+1];
		        //read file into buffer to be sent 
		        readFile(fileBuf, name); 
		        if(size < 4096){
		            n = write(sockfd2, fileBuf, BUFSIZE); 
		            if(n < 0 ) error("Error sending\n"); 
		        }
		        else{
		            rounds = (size + 4095) / 4096; 
		            int round_num = 0; 
		            for(i = 0; i<rounds; i++){
		                for(j = 0; j<4095; j++){
		                    currBuf[j] = fileBuf[round_num+j]; 
		                }
		                n = write(sockfd2, currBuf, BUFSIZE);  
		                round_num = round_num + 4096;
		                bzero(currBuf, BUFSIZE); 
		            }
		        }
		        n = write(sockfd2, buf, BUFSIZE); 
		        mhash(td, &fileBuf, 1); 
		        serverHash = mhash_end(td);
		        bzero(buf, BUFSIZE); 
		        sprintf(buf, "%s", serverHash); 
		        //printf("%s", serverHash); 
		        n = write(sockfd2, buf, BUFSIZE);// strlen(serverHash)); 
		        if(n<0) error("error sending!"); 
		        bzero(buf, BUFSIZE); 
		        n = read(sockfd2, buf, BUFSIZE); 
		        if(n<0) error("error reading!"); 
		        //printf("%s\n", buf); 
		    }

            } else if (strcmp(com, "UPL") == 0) {
                /* get start time*/
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
                
                /*SEND ACK TO CLIENT*/
                strcat(buf, "ready"); 
                
                n = write(sockfd2, buf, BUFSIZE); 
                if (n < 0){
                    error("Error in reading socket3\n"); 
                }
                bzero(buf, BUFSIZE); 

                /*Read filesize*/
                n = read(sockfd2, buf, BUFSIZE); 
                filelen = atoi(buf); 
                bzero(buf, BUFSIZE); 
                /*Loop to read in 4096 bit chunks*/
                rounds = (filelen + 4095) / 4096; 
                int round_num = 0;
                 
                for(i=0; i<rounds; i++){
                    n = read(sockfd2, buf, BUFSIZE); 
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

                n = read(sockfd2, buf, BUFSIZE);  
                mhash(td, &finalBuf, 1); 
                serverHash = mhash_end(td); 
                if(strcmp(serverHash, buf) == 0){
                    //printf("compared\n"); 
                }
                bzero(buf, BUFSIZE); 
                sprintf(buf, "Transfer was successful, throughput: %f microseconds", throughput); 
                n = write(sockfd2, buf, BUFSIZE); 
                if (n<0) error("Error writing\n"); 
            
            } else if (strcmp(com, "DEL") == 0) {
                bzero(buf, BUFSIZE);
                if(access(name, F_OK) != -1 || access(name, X_OK) != -1){
                    strcat(buf, "1"); 
                    n = write(sockfd2, buf, BUFSIZE); 
                    if (n<0) error("error sending"); 
                    bzero(buf, BUFSIZE); 
                    n = read(sockfd2, buf, BUFSIZE); 
                    bzero(buf, BUFSIZE); 
                    n = read(sockfd2, buf, BUFSIZE); 
                    if (strcmp(buf, "Yes") == 0){
                        remove(name);  
                        bzero(buf, BUFSIZE); 
                        strcat(buf, "1"); 
                        n = write(sockfd2, buf, BUFSIZE); 
                        if (n<0) error("error writing"); 
                    }
   
                } else{
                    strcat(buf, "-1");
                    n = write(sockfd2, buf, BUFSIZE); 
                    if (n< 0 ) error("error writing\n"); 
                    }

            } else if (strcmp(com, "MKD") == 0) {
                struct stat st = {0};

                if (stat(name, &st) == 0) {
                    if (S_ISDIR(st.st_mode)) {
                        // Return -2
                        bzero(buf, BUFSIZE);
                        strcat(buf, "-2");
                    } else {
                        int c = mkdir(name, 0700);
                        if (!c)
                            printf("Dir created");
                        // Return 1
                        bzero(buf, BUFSIZE);
                        strcat(buf, "1");
                    }                   
                } else {
                    // Return -1
                    bzero(buf, BUFSIZE);
                    strcat(buf, "-1");
                }

                n = write(sockfd2, buf, BUFSIZE);
                continue;

            } else if (strcmp(com, "RMD") == 0) {
                // Remove Directory
                int before = path_is_directory(name);

                if (!before) {
                    // not a directory, send -1
                    bzero(buf, BUFSIZE);
                    strcat(buf, "-1");
                    n = write(sockfd2, buf, BUFSIZE);
                } else {
                    // directory exists, send 1
                    bzero(buf, BUFSIZE);
                    strcat(buf, "1");
                    n = write(sockfd2, buf, BUFSIZE);

                    // Read in client confirmation
                    bzero(buf, BUFSIZE);
                    n = read(sockfd2, buf, BUFSIZE);
                    if (strcasecmp(buf, "yes")) {
                        int r = rmdir(name);
                        bzero(buf, BUFSIZE);
                        if (!r) {
                            strcat(buf, "1");
                        } else {
                            strcat(buf, "-1");
                        }
                        n = write(sockfd2, buf, BUFSIZE);
                        continue;
                    }
                }
                //n = write(sockfd2, buf, BUFSIZE);

            } else if (strcmp(com, "CHD") == 0) {
                bzero(buf, BUFSIZE);
                //printf("READ\n");

                n = read(sockfd2, buf, BUFSIZE);    // length of directory name
                if(n < 0)
                    error("Error reading from socket");
            
                //printf("read\n");
                len = atoi(buf);
                bzero(buf, BUFSIZE);

                n = read(sockfd2, buf, BUFSIZE);    // directory name
                if(n < 0)
                    error("Error reading from socket");
            
                //printf("DONE\n");
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

           // printf("\nN: %d\n",n);
            //printf("Recieved %s\n", buf);

            if(n < 0)
                error("Error reading from socket");
            //printf("server recieved %d bytes: %s\n" , n, buf);

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

int path_is_directory (const char* path) {
    struct stat s_buf;

    if (stat(path, &s_buf))
        return 0;

    return S_ISDIR(s_buf.st_mode);
}
