while ( (n = recv(connfd, buf, MAXLINE,0)) > 0)  {
   cout<<"String received from client: "<<buf;
   char *token, *dummy;
   dummy = buf;
   token = strtok(dummy, " "); // splits string by spaces

   if (strcmp("quit\n",buf)==0)  {
   	cout<<"The client has quit\n";
   }

   if (strcmp("ls\n",buf)==0)  {
	FILE *in;
	char temp[MAXLINE],port[MAXLINE]; 			// MAXLINE is Buf size
	int datasock;
	data_port = data_port + 1;

	if(data_port == atoi(argv[1])) {
		data_port = data_port + 1;
	}

	sprintf(port, "%d", data_port);
	datasock = create_socket(data_port);		//creating socket for data connection
	send(connfd, port, MAXLINE, 0);				//sending data connection port no. to client
	datasock = accept_conn(datasock);	 		//accepting connection from client

	if(!(in = popen("ls", "r"))){
		cout << "error" << endl;
	}

	while(fgets(temp, sizeof(temp), in) != NULL){
		send(datasock, "1", MAXLINE, 0);
		send(datasock, temp, MAXLINE, 0);
	}

	send(datasock, "0", MAXLINE, 0);
	pclose(in);
	//cout<<"file closed\n";
   }

   if (strcmp("cd", token) == 0) {
	token = strtok(NULL, " \n");
	cout << "Path given is: " << token << endl;
	if(chdir(token) < 0) {
		send(connfd, "0", MAXLINE, 0);
	}
	else{
		send(connfd, "1", MAXLINE, 0);
	}
   }
