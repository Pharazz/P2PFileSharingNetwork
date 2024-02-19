#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <sys/socket.h>                                                                            
#include <netinet/in.h>
#include <arpa/inet.h>
#include <strings.h>                                                           
#include <netdb.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/signal.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <strings.h>
#include <string.h>
#include <math.h>
#include <time.h>

time_t t;

#define	BUFSIZE 64
#define SERVER_TCP_PORT 3000	/* well-known port */
#define BUFLEN		256	/* buffer length */
#define PACKET_LEN	99
#define MAXTHREADNUM	10

#define	MSG		"Any Message \n"

int fSize(FILE * fp){
	fseek(fp, 0L, SEEK_END);
	int sz = ftell(fp);
	fseek(fp, 0L, SEEK_SET);
	return sz;
}

/*------------------------------------------------------------------------
 * main - UDP
 *------------------------------------------------------------------------
 */
 
 // struct to store PDU data
typedef struct pdu {
	char type;
	char data[100];
}PDU;

// struct to store child process information so that the top level parent can delete TCP children when deregistering
typedef struct {
	int port;
	int pid;
	char content[100];
}Thread;

// global array to store child processes
Thread childr[MAXTHREADNUM];

// declare helper functions
 int echod(int sd, char * contentname);
 void	reaper(int sig);
 int TCP();
 int echodrec(int sd, char * contentname, PDU spdu);
 int TCPrec(int portTCP, char * contentname, PDU spdu, char * AddressS);
 void closePID(char * contentname);
 void sendFilename( int s);

// main function
int
main(int argc, char **argv)
{

	while(1){
		// prepare for UDP socket creation
		char	*host = "localhost";
		int	port = 3000;
		int threadNum = 0;
		struct hostent	*phe;	/* pointer to host information entry	*/
		struct sockaddr_in sin;	/* an Internet endpoint address		*/
		int	s, n, type, alen;	/* socket descriptor and socket type	*/
		// instruct user to name this peer program
		printf("Tell me your name.\n");
		char peername[100];
		scanf("%s", peername);	
		// pull command line argument to correctly generate UDP connection		
		switch (argc) {
		case 1:
			break;
		case 2:
			host = argv[1];
		case 3:
			host = argv[1];
			port = atoi(argv[2]);
			break;
		default:
			fprintf(stderr, "usage: UDPtime [host [port]]\n");
			exit(1);
		}

		memset(&sin, 0, sizeof(sin));
		sin.sin_family = AF_INET;                                                                
		sin.sin_port = htons(port);
			                                                                        
	    /* Map host name to IP address, allowing for dotted decimal */
		if ( phe = gethostbyname(host) ){
			memcpy(&sin.sin_addr, phe->h_addr, phe->h_length);
		}
		else if ( (sin.sin_addr.s_addr = inet_addr(host)) == INADDR_NONE )
			fprintf(stderr, "Can't get host entry \n");
			                                                                
	    /* Allocate a socket */
		s = socket(AF_INET, SOCK_DGRAM, 0);
		if (s < 0)
			fprintf(stderr, "Can't create socket \n");
		
			                                                                
	    /* Connect the socket */
		if (connect(s, (struct sockaddr *)&sin, sizeof(sin)) < 0)
			fprintf(stderr, "Can't connect to %s %s \n", host, "Time");
		alen = sizeof(sin);
		int i;
		int j;
		/* generate buffers for program to run */
		printf("Issue a command.\n");
		char recieveData[101];
		char scannedtype[10];
		char contentname[100];
		char question[100];
		int portNumTCP[1];
		char 	intString[20];
		int debug = 0;
		struct pdu rpdu;
		struct pdu spdu;
		while(1){
			// get command in form of specifying PDU type from user
			bzero(spdu.data,sizeof(spdu.data));
			strcpy(spdu.data,"\0");
			printf("PDU type:\n");
			
			scanf("%s",scannedtype);
			spdu.type = scannedtype[0];
			switch(spdu.type){
				// user requested R type PDU to register content
				case 'R':
					// generate a randomized port number within a specified range that should be legal ports
					srand((unsigned)time(&t));
					portNumTCP[0] = (rand() % 40000)+10000;
					// request user input for content to be registered
					printf("Tell me the content.\n");
					scanf("%s", contentname);
					// check that content name is within legal limit
					if (strlen(contentname)<10){
						printf("Content name too big\n");	
						break;
					}
					// prepare R PDU data to be sent
					strcat(spdu.data,peername);
					strcat(spdu.data,"-");
					strcat(spdu.data,contentname);
					strcat(spdu.data,"-");
					sprintf(intString, "%d", portNumTCP[0]);
					strcat(spdu.data,intString);
					strcat(spdu.data,"-");
					printf("%c %s should be sent.\n",spdu.type,spdu.data);
					// send R type PDU to UDP index server
					(void) write(s, &spdu, strlen(spdu.data)+1);	
					// get the ACK or error
					char listenBufR[101];
					bzero(listenBufR,sizeof(listenBufR));
						if (recvfrom(s, listenBufR, sizeof(listenBufR), 0, (struct sockaddr *)&sin, &alen) < 0)
							fprintf(stderr, "recvfrom error\n");
						else {
							fprintf(stderr, "Received Ack or Error %s.\n",listenBufR);
							if(listenBufR[0]!='A'){
								//Received error
								printf("Not starting TCP.\n");
								printf("Peername already in use for this content enter new peername\n");
								scanf("%s", peername);
								break;
							}
							else{
								//Received Acknowledgement
								
							}
					}
					// prepare to create child process to hold TCP port for other peers to download content from
					strcpy(childr[threadNum].content, contentname);				
					childr[threadNum].port = portNumTCP[0];
					// create child process
					childr[threadNum].pid = fork();					
					switch (childr[threadNum].pid){
			 			case 0: 
			 				/* child creates TCP Port and starts to listen for connections*/
							TCP(portNumTCP[0],contentname);	
			  			default:
			  				// parent stores child process information and loops back to recieve another command from user
			  				printf("PID port and content of child: %d %d %s\n", childr[threadNum].pid, childr[threadNum].port, childr[threadNum].content);
			  				threadNum++;
							continue;
			  			case -1:
			  				// error recovery
			  				strcpy(childr[threadNum].content,"");
							childr[threadNum].port = 0;
							fprintf(stderr, "fork: error\n");
							break;				
						break;
					}
				case 'S':
					// user has requested to download content so get name of content
					printf("Tell me the content.\n");
					scanf("%s", contentname);
					// prepare to send S PDU to index server
					strcat(spdu.data,peername);
					strcat(spdu.data,"-");
					strcat(spdu.data,contentname);
					strcat(spdu.data,"-");
					printf("%c %s is sent to index server.\n",spdu.type,spdu.data);
					char listenBufS[101];
					bzero(listenBufS,sizeof(listenBufS));
					// send S PDU
					(void) write(s, &spdu, strlen(spdu.data)+1);
					// listen for index server response	
					if (recvfrom(s, listenBufS, sizeof(listenBufS), 0, (struct sockaddr *)&sin, &alen) < 0)
							fprintf(stderr, "recvfrom error\n");	
					listenBufS[101]='\0';
					printf("%s buf.\n",listenBufS);
					switch(listenBufS[0]){
						case 'S':
							// index server responded with S PDU so download can commence
							printf("Case S.\n");
							bzero(spdu.data,sizeof(spdu.data));
							// prepare to create child process that can initiate TCP connection and download content based on index server response
							char PeerS[100];
							char AddressS[100];
							char PortS[100];
							int track1S = 0;
							int track2S = 0;
							int peerlenS = 0;
							int adrlenS = 0;
							int portlenS = 0;
							for(i=0;i<100;i++){
					    			PeerS[i]='\0';
					    		}
					    		for(i=0;i<100;i++){
					    			AddressS[i]='\0';
					    		}
					    		for(i=0;i<100;i++){
				    				PortS[i]='\0';
					    		}
							for(i=0;i<101;i++){
					    			if(track1S==0){
					    				PeerS[peerlenS]=listenBufS[i+1];
					    				if(PeerS[peerlenS]=='-'){
					    					PeerS[peerlenS]='\0';
					    					track1S=1;
					    					
					    				}
					    				peerlenS++;
					    			}
					    			else if(track2S==0){
					      				AddressS[adrlenS]=listenBufS[i+1];
					    				if(AddressS[adrlenS]=='-'){
					    					AddressS[adrlenS]='\0';
					    					track2S=1;
					    				}
					    				adrlenS++;      			
					    			}
					    			else{
					    				PortS[portlenS]=listenBufS[i+1];
					    				if(PortS[portlenS]=='-'){
					    					PortS[portlenS]='\0';
					    					break;
					    				}
					    				portlenS++;
					    			}
			    				}
			    				printf("Received %s %s %s to now try to download file.\n", PeerS, AddressS, PortS);	

							strcpy(spdu.data,PeerS);
							strcat(spdu.data,"-");
							strcat(spdu.data, contentname);
							strcat(spdu.data,"-");
							// check that peer is not attempting to download file from itself if only peer registered
							if (!(strcmp(PeerS, peername))){
								printf("You already have this file!\n");
								break;
							}
							int portSint = atoi(PortS);
							// prepare to generate child process for TCP download of content
							strcpy(childr[threadNum].content, contentname);
							childr[threadNum].port = portNumTCP[0];
							// generate child process and store information
							childr[threadNum].pid = fork();
							switch (childr[threadNum].pid){
					 			case 0:		
					 				/* child process creates TCP port, connects to content provider and downloads content*/
									TCPrec(portSint,contentname,spdu,AddressS);
									// done downloading	
									printf("Downloaded file done. I am now going to be a host.\n");
									sleep(1);
									// make myself a host by sending R PDU
									bzero(spdu.data,sizeof(spdu.data));
									spdu.type='R';
									// generate random port number in legal range of port numbers
									srand((unsigned)time(&t));
									int randum = (rand() % 40000)+10000;					
									fprintf(stderr,"Register content at port %d.\n", randum);
									// prepare R PDU
									strcat(spdu.data,peername);
									strcat(spdu.data,"-");
									strcat(spdu.data,contentname);
									strcat(spdu.data,"-");
									sprintf(intString, "%d", randum);
									strcat(spdu.data,intString);
									strcat(spdu.data,"-");
									fprintf(stderr,"Sending registration PDU %c %s.\n",spdu.type,spdu.data);
									// send RPDU to index server
									(void) write(s, &spdu, strlen(spdu.data)+1);
									// get the ACK or error
									bzero(listenBufS,sizeof(listenBufS));
									if (recvfrom(s, listenBufS, sizeof(listenBufS), 0, (struct sockaddr *)&sin, &alen) < 0)
										fprintf(stderr, "recvfrom error\n");
									else {
										fprintf(stderr, "Received Ack or Error %s.\n",listenBufS);
										if(listenBufS[0]!='A'){
											printf("Not making TCP.\n");
											break;
										}
									}
									// create TCP socket and listen for connections to allow others to download from this child process
									fprintf(stderr, "Making a socket to register downloaded content\n");
									TCP(randum,contentname);
									break;
					  			default:
					  				// parent process stores child information and continues operation
					  				printf("PID port and content of child: %d %d %s\n", childr[threadNum].pid, childr[threadNum].port, childr[threadNum].content);
					  				printf("Continuing operation.\n");
					  				threadNum++;
					  				printf("There are now %d TCP ports open.\n",threadNum);
									continue;
					  			case -1:
					  				// error recovery
									fprintf(stderr, "fork: error\n");
									break;				
								break;
							}
						printf("Finalizing operation.\n");
						break;	
					default:
						printf("Error in syntax command\n");
						break;													
					}
					break;	
				case 'O':
					// user has requested list of content
					printf("Case O\n");
					printf("Requesting content list.\n");
					// prepare O type PDU
					spdu.type='O';
					bzero(spdu.data,sizeof(spdu.data));
					strcat(spdu.data,"Send list of content\n");
					bzero(listenBufS,sizeof(listenBufS));
					// send to index server
					(void) write(s, &spdu, strlen(spdu.data)+1);	
					// get response or error and display for user
					if (recvfrom(s, listenBufS, sizeof(listenBufS), 0, (struct sockaddr *)&sin, &alen) < 0)
							fprintf(stderr, "recvfrom error\n");
					else{
						for(i=0;i<100;i++){
							rpdu.data[i]=listenBufS[i+1];
						}
						printf("List of registered content:\n%s", rpdu.data);
					}
					break;
					
				case 'T':
					// user has requested to deregister content
					printf("Case T\n");
					// get user input about content
					printf("What content would you like to deregister\n");
					scanf("%s", contentname);
					// prepare T type PDU
					spdu.type = 'T';
					bzero(spdu.data,sizeof(spdu.data));
					strcpy(spdu.data,peername);
					strcat(spdu.data,"-");
					strcat(spdu.data,contentname);
					strcat(spdu.data,"-");
					printf("%c %s should be sent.\n",spdu.type,spdu.data);
					// send T PDU to index server
					(void) write(s, &spdu, strlen(spdu.data)+1);	
					// get the ACK or error
					bzero(listenBufR,sizeof(listenBufR));
						if (recvfrom(s, listenBufR, sizeof(listenBufR), 0, (struct sockaddr *)&sin, &alen) < 0)
							fprintf(stderr, "recvfrom error\n");
						else {
							fprintf(stderr, "Received Ack or Error %s.\n",listenBufR);
							if(listenBufR[0]!='A'){
								//Received error
								printf("Not able to deregister\n");
								break;
							}
							else{
								printf("Attempting to close Socket for %s\n", contentname);
								closePID(contentname);
								printf("Finished closing socket\n");
								//Received Acknowledgement
								
							}
						printf("Finishing ack processing\n");
						}
					break;
				case 'Q':
					// user has requested to quit - check they entered command correctly
					if (strcmp(scannedtype, "Quit")){
						printf("Invalid input, if you wish to Quit type Quit\n");
						break;
					}		
					printf("case Quit\n");
					// for each child process (holding an open TCP port)
					for(int k = 0; k<MAXTHREADNUM; k++){
						// if there is a valid child process entry stored in this parent global array
						if(strcmp(childr[k].content, "")){
							// prepare a T type PDU and send to index server
							spdu.type = 'T';
							bzero(spdu.data,sizeof(spdu.data));
							strcpy(spdu.data,peername);
							strcat(spdu.data,"-");
							strcat(spdu.data,childr[k].content);
							strcat(spdu.data,"-");
							printf("%c %s should be sent.\n",spdu.type,spdu.data);
							// send to index server
							(void) write(s, &spdu, strlen(spdu.data)+1);	
							// get the ACK or error
							bzero(listenBufR,sizeof(listenBufR));
								if (recvfrom(s, listenBufR, sizeof(listenBufR), 0, (struct sockaddr *)&sin, &alen) < 0)
									fprintf(stderr, "recvfrom error\n");
								else {
									fprintf(stderr, "Received Ack or Error %s.\n",listenBufR);
									if(listenBufR[0]!='A'){
										//Received error
										printf("Not able to deregister\n");
									}
									else{
										// close the child process TCP that the index server has deregistered
										printf("Attempting to close Socket for %s\n", childr[k].content);
										closePID(childr[k].content);
										printf("Finished closing socket\n");
										//Received Acknowledgement
										
									}
								printf("Finishing ack processing\n");
								}
						}
					}
					break;
				default:
					// catch all case that loops back from illegal commands
					printf("Should not get here All case.\n");
					break;
					
			}
		}	
	}	
}

// a method to create a TCP port to listen for clients that want to download content
int TCP(int portTCP, char * contentname){
	// TCP port creation preamble
	int 	sd, new_sd, client_len;
	struct	sockaddr_in server, client;
	FILE *fptr;
	char * file;
	
	/* Create a stream socket	*/	
	if ((sd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		fprintf(stderr, "Can't creat a socket\n");
		exit(1);
	}

	/* Bind an address to the socket	*/
	bzero((char *)&server, sizeof(struct sockaddr_in));
	server.sin_family = AF_INET;
	server.sin_port = htons(portTCP);
	server.sin_addr.s_addr = htonl(INADDR_ANY);
	if (bind(sd, (struct sockaddr *)&server, sizeof(server)) == -1){
		fprintf(stderr, "Can't bind name to socket\n");
		exit(1);
	}

	/* queue up to 5 connect requests  */
	listen(sd, 5);

	(void) signal(SIGCHLD, reaper);

	while(1) {
		// client detected
		  client_len = sizeof(client);
		  new_sd = accept(sd, (struct sockaddr *)&client, &client_len);
		  if(new_sd < 0){
		    fprintf(stderr, "Can't accept client \n");
		    exit(1);
		  }
		  // create child process to handle the content download
		  switch (fork()){
		  case 0:		/* child */
			(void) close(sd);
			fprintf(stderr,"Connected TCP port.\n");
			// trigger child process to start sending files to connected peer
			exit(echod(new_sd,contentname));
		  default:		/* parent */
		  	// close the new connection after the TCP download is complete and keep this process running for new connections
		  	fprintf(stderr,"Returning from TCP port creation.\n");
			(void) close(new_sd);
			break;
		  case -1:
			fprintf(stderr, "fork: error\n");
		  }
	}
}

// a method to help close child processes (to kill TCP ports when deregistering)
void closePID(char * contentname){
	printf("Checking %s.\n",contentname);
	int verdict = 0;
	// find the content that is trying to be closed
	for(int i=0; i<MAXTHREADNUM; i++) {
		if (!(strcmp(childr[i].content, contentname))){
			// found it so close it
			verdict = 1;
			printf("Closing socket %s with pid: %d\n",contentname, childr[i].pid);
			kill(childr[i].pid, SIGTERM);
			(void) signal(SIGCHLD, SIG_IGN);
			printf("Closed PID:%d\n", childr[i].pid);
			strcpy(childr[i].content,"");
			childr[i].port = 0;
			childr[i].pid = 0;
			break;
		}
	}
	printf("Finished closing PID\n");
}

// a method to recieve TCP downloaded content
int TCPrec(int portTCP, char * contentname, PDU spdu, char * AddressS){
	fprintf(stderr,"Attempting to download %s.\n",contentname);
	fprintf(stderr,"Trying to connect to %s.\n",AddressS);

	// TCP port creation preample
	int 	n, i, bytes_to_read;
	int 	sd, port;
	port = portTCP;
	struct	hostent		*hp;
	struct	sockaddr_in server;
	char	*host, *bp, rbuf[BUFLEN], sbuf[PACKET_LEN], sbuf2[PACKET_LEN];
	host = AddressS;
	// open file to receive downloaded content
	FILE *fp;
	
	/* Create a stream socket	*/	
	if ((sd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		fprintf(stderr, "Can't creat a socket\n");
		exit(1);
	}

	bzero((char *)&server, sizeof(struct sockaddr_in));
	server.sin_family = AF_INET;
	server.sin_port = htons(port);
	if (hp = gethostbyname(host)) 
	  bcopy(hp->h_addr, (char *)&server.sin_addr, hp->h_length);
	else if ( inet_aton(host, (struct in_addr *) &server.sin_addr) ){
	  fprintf(stderr, "Can't get server's address\n");
	  exit(1);
	}

	/* Connecting to the server */
	if (connect(sd, (struct sockaddr *)&server, sizeof(server)) == -1){
	  fprintf(stderr, "Can't connect \n");
	  exit(1);
	}
	// trigger TCP download
	echodrec(sd,contentname,spdu);
}


/*	reaper to cleanup resources after closing child processes		*/
void	reaper(int sig)
{
	int	status;
	while(wait3(&status, WNOHANG, (struct rusage *)0) >= 0);
}

// a method to send TCP download files to connected client
int echod(int sd, char * contentname){
	// file sending preamble
	FILE *fptr;
	char 	intString[20];
	char	*bp, buf[BUFLEN], sbuf[PACKET_LEN], sbuf2[PACKET_LEN+1], sbuf3[PACKET_LEN+1];
	int 	n, bytes_to_read;
	bzero(sbuf3, PACKET_LEN+1);
	for (int p = 0; p<20; p++){
		sbuf3[p] = '\0';
	}
	// wait for connected client to send D type PDU
	n=read(sd, sbuf3, PACKET_LEN+1);
	fprintf(stderr, "Received %s PDU\n", sbuf3);
		
	fprintf(stderr, "Sending %s.\n", contentname);
	// open file to be sent
	fptr = fopen(contentname, "r");
	if(fptr != NULL){
		fprintf(stderr, "Opened %s.\n", contentname);
		// calculate how many packets need to be sent (there is a maximum packet size)
		int file_Size = fSize(fptr);
		int packetDist =  file_Size / PACKET_LEN;
		if (file_Size % PACKET_LEN != 0){
			packetDist++;
		}
		fprintf(stderr, "Sending %d packets.\n", packetDist);

		sprintf(intString, "%d", packetDist);
		// tell client how many packets will be sent
		write(sd, intString, sizeof(intString));
		sleep(2); 
		//send number of packets
		for(int i = 0; i< packetDist; i++){
			// send packets as C type PDU
			sbuf2[0] = 'C';
			sbuf2[1] = '\0';
			char flag[file_Size];
			fread(sbuf, sizeof(char), PACKET_LEN, fptr);
			strcat(sbuf2,sbuf);
			fprintf(stderr, "Packet %d Data being sent %s\n",i,sbuf2);
			write(sd, sbuf2, PACKET_LEN+1);
			// prepare for next packet
			bzero(sbuf, PACKET_LEN);
			bzero(sbuf2, PACKET_LEN+1);
			sleep(2);
			fseek(fptr, 0L, 100);
			fprintf(stderr, "Packet %d sent.\n", i); 		
		}
		// close the file
		fclose(fptr);
	}
	else{	// handle errors
		fprintf(stderr, "\n File %s not found", buf);
		write(sd, "?File not found\n", sizeof("?File not found\n"));
	}	
	// close the socket when sending packets are finished
	close(sd);

	return(0);
}

// a method to receive downloaded content on TCP port
int echodrec(int sd, char * contentname, PDU spdu){
	fprintf(stderr,"Attempting to download file.\n");
	// file receive preamble
	FILE *fptr;
	char 	intString[20];
	char	*bp, buf[BUFLEN], sbuf[PACKET_LEN+1], sbuf2[PACKET_LEN+1], sbuf3[PACKET_LEN+1];
	bzero(sbuf,sizeof(sbuf));
	strcat(sbuf,contentname);
	int 	n, bytes_to_read;
	char ecbuf[100];
	FILE * fp;
	// open file to place content
	fp = fopen(sbuf, "w");
	  strcpy(sbuf3, "D");
	  strcat(sbuf3, contentname);
	  // send D type PDU to trigger download
	  fprintf(stderr, "Sending PDU: %s\n", sbuf3);
	  write(sd, sbuf3, PACKET_LEN+1);
	  while (n=read(sd, sbuf, 1)){ 
	  	if (sbuf[0] == '?'){
	  		fprintf(stderr,"This data is corrupted or not available\n");
	  		read(sd, sbuf2, PACKET_LEN);
	  		fprintf(stderr, "Error message from server: %s\n",sbuf2);
	  		break;
	  	}
	  	// if not error
	  	int packetnum = atoi(sbuf);
	  	char file[(packetnum*PACKET_LEN)+1];
	  	bzero(file, sizeof(file));
	  	file[0] = '\0';
	  	bzero(sbuf, sizeof(sbuf));
	  	// for all packets, obtain and store them, remove the C from the C type PDU received
	  	for(int i = 0; i< packetnum; i++){
	  		while(n = read(sd, sbuf, PACKET_LEN+1)){
		  		for (int k = 0; k < PACKET_LEN; k++){
		  			sbuf2[k] = sbuf[k+1];
		  		}
		  		sbuf2[PACKET_LEN] = '\0';
		  		if (sbuf[0] == 'C'){
		  			// place received data into buffer to be placed into file
		  			strcat(file, sbuf2);
		  		}
		  		else if(sbuf[0] == '?'){
		  			fprintf(stderr, "This data is corrupted or not available\n");
		  		fprintf(stderr, "Error message from server: %s\n",sbuf2);
		  		}		  		
	  		}
	  	}
	  	fprintf(stderr, "file %s downloaded: ",contentname);
	  	fprintf(stderr, "File content: %s", file);
	  	// place buffered received data into file
	  	fprintf(fp, "%s", file);
	  	
	  }
	  // close file and connection when done downloading
	  fclose(fp);

	close(sd);
	return(0);
}
