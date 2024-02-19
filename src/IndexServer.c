#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <unistd.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <stdio.h>
#include <time.h>
#include <strings.h>
#include <math.h>
#include <arpa/inet.h>
#define PACKET_LEN 100
#define PEERSIZE 3
#define INDEXSIZE 3
#define FLAGVERDICT 10
#define MAXSIZE 5

// helper functions
const char* callLS( );
int fSize(FILE * fp);
int checkNameC(char * Contentname);
int checkNameP(char * Peername, int index);
void printStructs();
int placeSourceNull(int dex);
int placeIndexNull();
int findPeeratIndex(char * peername,int dex);
int getContextIndex(char * Contentname);
int getUnusedPeer(int dex);
int checkEmpty(int dex);
/*------------------------------------------------------------------------
 * main - Iterative UDP server
 *------------------------------------------------------------------------
 */
 
 // struct to store peers with connection information
typedef struct {
	char peername[100];
	char port[100];
	char address[100];
	int lastUsed;
}Peer;

// struct to store content with registered peers
typedef struct {
	char contentName[100];
	//char* contentName;
	Peer sources[PEERSIZE];
}Index;

// global array to store content
Index content[10];
 
// main function
int
main(int argc, char *argv[])
{
	// UDP socket preamble	
	struct  sockaddr_in fsin;	/* the from address of a client	*/
	char	buf[101];			/* "input" buffer; any size > 0	*/
	char    *pts;
	int	sock;					/* server socket		*/
	time_t	now;				/* current time			*/
	int	alen;					/* from-address length		*/
	struct  sockaddr_in sin; 	/* an Internet endpoint address         */
    	int     s, type;        	/* socket descriptor and socket type    */
	int 	port=3000;
	// struct to handle UDP messages
	struct pdu {
		char type;
		char data[100];
	};
	// initialize content array	
	for (int k = 0; k<INDEXSIZE; k++){
		strcpy(content[k].contentName, "");
		for (int t =0; t<PEERSIZE; t++){
			strcpy(content[k].sources[t].peername, "");
			strcpy(content[k].sources[t].port, "");
			strcpy(content[k].sources[t].address, "");
			content[k].sources[t].lastUsed = 0;
		}
	}
	// get command line arguments to setup UDP socket
 	 switch(argc){
		case 1:
			break;
		case 2:
			port = atoi(argv[1]);
			break;
		default:
			fprintf(stderr, "Usage: %s [port]\n", argv[0]);
			exit(1);
	}    
	
	memset(&sin, 0, sizeof(sin));
        sin.sin_family = AF_INET;
        sin.sin_addr.s_addr = INADDR_ANY;
        sin.sin_port = htons(port);
                                                                                                 
    /* Allocate a socket */
        s = socket(AF_INET, SOCK_DGRAM, 0);
        if (s < 0)
		fprintf(stderr, "can't creat socket\n");
                                                                                
    /* Bind the socket */
        if (bind(s, (struct sockaddr *)&sin, sizeof(sin)) < 0)
		fprintf(stderr, "can't bind to %d port\n",port);
        listen(s, 5);	
	alen = sizeof(fsin);   
	
	while (1) {
		// receive a PDU
		bzero(buf,sizeof(buf));
		if (recvfrom(s, buf, sizeof(buf), 0,
				(struct sockaddr *)&fsin, &alen) < 0)
			fprintf(stderr, "recvfrom error\n");
        	// prepare to operate based on PDU
	    	struct pdu rpdu;
	    	struct pdu spdu;
	    	int k;
	    	char assignPDU[101];
	    	const char * lsResult;
	    	int loop = 0;
		int i;
		char sendLS[101];
		FILE *fptr;
		// legal PDU?
		if(buf[0] == 'R' || buf[0] == 'S' || buf[0] == 'T' || buf[0] == 'O' || buf[0] =='E'){
			for(k=0;k<100;k++)assignPDU[k] = buf[k+1];
			assignPDU[101]='\0';
			strcpy(rpdu.data,assignPDU);
			rpdu.type = buf[0];
			printf("Recieved PDU %c %s\n", rpdu.type, rpdu.data);        	
        	}
		else {
			printf("Error PDU type %c received\n", buf[0]);
		}
		// operation preamble
		char Peer[100];
		char ContentName[100];
	    	char Address[100];	
	    	int track1 = 0;
	    	int track2 = 0;
	    	int peerLen = 0;
	    	int ContLen = 0;
	    	int AddrLen = 0;	
        	switch(rpdu.type){
        	
		case 'R':
			// received R type PDU so prepare to register content
			printf("Case R \n");
	    		for(i=0;i<100;i++){
	    			Peer[i]='\0';
	    		}
	    		for(i=0;i<100;i++){
	    			ContentName[i]='\0';
	    		}
	    		for(i=0;i<100;i++){
    				Address[i]='\0';
	    		}
	    		for(i=0;i<100;i++){
	    			if(track1==0){
	    				Peer[peerLen]=rpdu.data[i];
	    				if(Peer[peerLen]=='-'){
	    					Peer[peerLen]='\0';
	    					track1=1;
	    					
	    				}
	    				peerLen++;
	    			}
	    			else if(track2==0){
	      				ContentName[ContLen]=rpdu.data[i];
	    				if(ContentName[ContLen]=='-'){
	    					ContentName[ContLen]='\0';
	    					track2=1;
	    				}
	    				ContLen++;      			
	    			}
	    			else{
	    				Address[AddrLen]=rpdu.data[i];
	    				if(Address[AddrLen]=='-'){
	    					Address[AddrLen]='\0';
	    					break;
	    				}
	    				AddrLen++;
	    			}
	    		}
	    		
	    		printf("Received R PDU from Peer %s with Content %s on Port %s.\n", Peer, ContentName, Address);
	    		
	    		// if new content
			if(!(checkNameC(ContentName))){
					// find where to store it
					int indexplace = placeIndexNull();
					printf("Content being registered\n");
					// register it
					strcpy(content[indexplace].contentName, ContentName);
					strcpy(content[indexplace].sources[0].peername, Peer);
					strcpy(content[indexplace].sources[0].port, Address);
					char * receivedAddr = inet_ntoa(fsin.sin_addr);				
					strcpy(content[indexplace].sources[0].address, receivedAddr);
					// prepare to send Ack
					spdu.type='A';
					strcpy(spdu.data, content[indexplace].contentName);
					strcat(spdu.data, "-");
					strcat(spdu.data, content[indexplace].sources[0].peername);
					strcat(spdu.data, "-");
					strcat(spdu.data, content[indexplace].sources[0].port);
					strcat(spdu.data, "-");
					printf("Ack new content %s\n",spdu.data);
					// send Ack
					(void) sendto(s, &spdu, strlen(spdu.data)+1, 0, (struct sockaddr *)&fsin, sizeof(fsin));
					// update terminal for demo purposes
					printStructs();
			}
			// if old content
			else if(checkNameC(ContentName)){
				// find index
				int Condex = getContextIndex(ContentName);
				// if new peer
				if(!(checkNameP(Peer,Condex))){	
					// prepare to register peer
					int peerdex = placeSourceNull(Condex);
					// register peer
					strcpy(content[Condex].sources[peerdex].peername, Peer);
					strcpy(content[Condex].sources[peerdex].port, Address);
					char * receivedAddr = inet_ntoa(fsin.sin_addr);				
					strcpy(content[Condex].sources[peerdex].address, receivedAddr);
					// prepare Ack
					spdu.type='A';
					strcpy(spdu.data, content[Condex].contentName);
					strcat(spdu.data, "-");
					strcat(spdu.data, content[Condex].sources[peerdex].peername);
					strcat(spdu.data, "-");
					strcat(spdu.data, content[Condex].sources[peerdex].port);
					strcat(spdu.data, "-");
					printf("Ack old content new peer %s\n",spdu.data);
					// send Ack
					(void) sendto(s, &spdu, strlen(spdu.data)+1, 0, (struct sockaddr *)&fsin, sizeof(fsin));
					// update terminal for demo purposes
					printStructs();	
				}
				// if old peer
				else{
					printf("This content is already registered by this peer\n");
					spdu.type='E';
					strcpy(spdu.data,"This content is already registered by this peer");
					// send error PDU
					(void) sendto(s, &spdu, strlen(spdu.data)+1, 0, (struct sockaddr *)&fsin, sizeof(fsin));					
				}
			}	
			else {
				printf("Catch all for bad input\n");	
			}
			
			break;
			
		case 'T':
			// received T PDU to deregister content
			printf("Case T\n");
			// prepare to extract information from PDU
	    		for(i=0;i<100;i++){
	    			Peer[i]='\0';
	    		}
	    		for(i=0;i<100;i++){
	    			ContentName[i]='\0';
	    		}
	    		for(i=0;i<100;i++){
    				Address[i]='\0';
	    		}
	    		for(i=0;i<100;i++){
	    			if(track1==0){
	    				Peer[peerLen]=rpdu.data[i];
	    				if(Peer[peerLen]=='-'){
	    					Peer[peerLen]='\0';
	    					track1=1;
	    					
	    				}
	    				peerLen++;
	    			}
	    			else if(track2==0){
	      				ContentName[ContLen]=rpdu.data[i];
	    				if(ContentName[ContLen]=='-'){
	    					ContentName[ContLen]='\0';
	    					track2=1;
	    				}
	    				ContLen++;      			
	    			}
	    			else{
	    				Address[AddrLen]=rpdu.data[i];
	    				if(Address[AddrLen]=='-'){
	    					Address[AddrLen]='\0';
	    					break;
	    				}
	    				AddrLen++;
	    			}
	    		}
	    		
	    		printf("\nReceived T PDU to delete Peer %s that owns Content %s with Port %s\n", Peer, ContentName, Address);
	    		
	    		// if content is not present
	    		if(!(checkNameC(ContentName))){
	    			printf("Content not registered.\n");
	    			char noConBuf[100];
	    			bzero(noConBuf,sizeof(noConBuf));
	    			strcpy(noConBuf,"EContent not registered.");
	    			// send error PDU that content is not registered
				(void) sendto(s, &noConBuf, strlen(noConBuf)+1, 0, (struct sockaddr *)&fsin, sizeof(fsin));	    			
	    		}				
	    		else {
	    			// we found content to be deregistered
	    			int removeCon = getContextIndex(ContentName);
	    			// check if peer is registered for this content
	    			if(!(checkNameP(Peer,removeCon))){	
	    				// send error PDU	
	    				printf("Peer not registered for this content to remove them.\n");
					spdu.type='E';
					strcpy(spdu.data,"Peer not registered for this content to remove them.\n");
					(void) sendto(s, &spdu, strlen(spdu.data)+1, 0, (struct sockaddr *)&fsin, sizeof(fsin));
	    			}
	    			else{	
	    			// peer is registered for content to be deregistered
	    				int removePer = findPeeratIndex(Peer, removeCon);	
	    				// remove the peer from the content
	    				strcpy(content[removeCon].sources[removePer].peername,"");
	    				strcpy(content[removeCon].sources[removePer].port,"");
	    				strcpy(content[removeCon].sources[removePer].address,"");	
	    				printf("Removed peer from content.\n");
	    				// send Ack to let peer know
	    				spdu.type='A';
					strcpy(spdu.data,"Removed peer");
					(void) sendto(s, &spdu, strlen(spdu.data)+1, 0, (struct sockaddr *)&fsin, sizeof(fsin));
					checkEmpty(removeCon);
					// update terminal for demo purposes
					printStructs();
	    			}
	    		}
	    		
	    		break;
	    		
	    	case 'S':
	    		// received S type PDU to download content
			printf("Case S\n");	
			// prepare to extract info from PDU
	    		for(i=0;i<100;i++){
	    			Peer[i]='\0';
	    		}
	    		for(i=0;i<100;i++){
	    			ContentName[i]='\0';
	    		}
	    		for(i=0;i<100;i++){
    				Address[i]='\0';
	    		}
	    		for(i=0;i<100;i++){
	    			if(track1==0){
	    				Peer[peerLen]=rpdu.data[i];
	    				if(Peer[peerLen]=='-'){
	    					Peer[peerLen]='\0';
	    					track1=1;
	    					
	    				}
	    				peerLen++;
	    			}
	    			else if(track2==0){
	      				ContentName[ContLen]=rpdu.data[i];
	    				if(ContentName[ContLen]=='-'){
	    					ContentName[ContLen]='\0';
	    					track2=1;
	    				}
	    				ContLen++;      			
	    			}
	    		}
	    		printf("\nReceived S PDU from Peer %s requesting Content %s\n", Peer, ContentName);
	    		// if content doesnt exist
			if(!(checkNameC(ContentName))){
				// generate and send error PDU
				int indexplace = placeIndexNull();
				printf("No content exists in index for this request.\n");
				spdu.type='E';
				strcpy(spdu.data,"No content exists in index for this request.\n");
				(void) sendto(s, &spdu, strlen(spdu.data)+1, 0, (struct sockaddr *)&fsin, sizeof(fsin));
			}
			else {	
				// else generate S PDU that instructs peer how to download content
				printf("Preparing to direct content.\n");
				int findCon = getContextIndex(ContentName);
				int findPer = getUnusedPeer(findCon);
				bzero(spdu.data,sizeof(spdu.data));
				spdu.type = 'S';
				strcpy(spdu.data,content[findCon].sources[findPer].peername);
				strcat(spdu.data,"-");
				strcat(spdu.data,content[findCon].sources[findPer].address);
				strcat(spdu.data,"-");
				strcat(spdu.data,content[findCon].sources[findPer].port);
				strcat(spdu.data,"-");
				printf("Sending directed content %c %s\n",spdu.type,spdu.data);
				// send S PDU that contains peer name, IP address and port
				(void) sendto(s, &spdu, strlen(spdu.data)+1, 0, (struct sockaddr *)&fsin, sizeof(fsin));
				// update terminal for demo purposes
				printStructs();
			}
			
			break;
			
		case 'O':
			// received O PDU to list content
			printf("Case O\n");
			bzero(spdu.data,sizeof(spdu.data));
			strcpy(spdu.data,"\0");
			// prepare to send list of content registered
			spdu.type = 'O';
			// generate list of content in O PDU data
			for(int i=0;i<INDEXSIZE;i++){
				if((strcmp(content[i].contentName,""))){
					strcat(spdu.data,content[i].contentName);
					strcat(spdu.data,"\n");
				}
			}
			printf("Sending: %c %s",spdu.type,spdu.data);
			//send O PDU with list of content
			(void) sendto(s, &spdu, strlen(spdu.data)+1, 0, (struct sockaddr *)&fsin, sizeof(fsin));
		
		default :	
			// catch all to catch weird inputs and loop back for new input
			printf("Default.\n");
			break;	
		}
	}
                                                               
}

// check peer list at content index for peer with peer name
int checkNameP(char * Peername, int index){
	int verdict = 0;
	// for all peers
	for (int j = 0; j<PEERSIZE; j++){
		if (!(strcmp(content[index].sources[j].peername, Peername))){
			// if we found peer at this index, return 1, else return 0
			verdict = 1;
		}	
	}
	return verdict;
}

// check content list for content with content name
int checkNameC(char * Contentname){
	printf("Checking %s.\n",Contentname);
	int verdict = 0;
	// for all content
	for(int i=0; i<INDEXSIZE; i++) {
		if (!(strcmp(content[i].contentName, Contentname))){
			// if we found the content that matches the content name requested return 1 else return 0
			verdict = 1;
			printf("Found %s.\n",Contentname);
		}
	}
	return verdict;
}

// get content index of content that matches content name, only called when content is already determined in registration
int getContextIndex(char * Contentname){
	printf("Looking for index of %s.\n",Contentname);
	int verdict = 0;
	// for all content
	for(int i=0; i<INDEXSIZE; i++) {
		if (!(strcmp(content[i].contentName, Contentname))){
			// if we found content return the index it is at
			verdict = i;
			i = (INDEXSIZE+1);
			printf("Found index of %s.\n",Contentname);
		}
	}
	return verdict;
}

// find first null index for placing new peers
int placeIndexNull(){	
	int verdict = FLAGVERDICT;
	// for all content
	for(int i=0; i<INDEXSIZE; i++) {
		if(!(strcmp(content[i].contentName,""))){
			// if we found content that is empty return the index to place content at this index, else return FLAG value
			verdict = i;
			i= (INDEXSIZE+11);
		}
	}
	// catch if we are out of space for content
	if(verdict == FLAGVERDICT){
		printf("Out of space for Content (index).\n");
		// return -1 to indicate this error
		verdict = -1;
	}
	return verdict;
}

// find peer at index of specific content, only called when peer is determined to be in the array at this content
int findPeeratIndex(char * peername,int dex){
	int verdict = FLAGVERDICT;
	// for all peers at this content
	for(int i=0; i<PEERSIZE; i++) {
		if(!(strcmp(content[dex].sources[i].peername,peername))){
			// if we have found the peer return the index
			verdict = i;
			i= (PEERSIZE+11);
		}
	}
	// if we did not find the peer
	if(verdict == FLAGVERDICT){
		printf("Did not find peer.\n");
		// return -1
		verdict = -1;
	}
	return verdict;
}

// find null peer at index to place new peer
int placeSourceNull(int dex){
	int verdict = FLAGVERDICT;
	// for all peers at this content
	for (int j = 0; j<PEERSIZE; j++){
		if(!(strcmp(content[dex].sources[j].peername,""))){
			// if we found an empty slot return that index
			verdict = j;
			j = (PEERSIZE+1);			
		}	
	}
	if(verdict == FLAGVERDICT){
		// else return -1 to indicate no more space for peers
		printf("Out of space for Peers (at this content).\n");
		verdict = -1;
	}
	return verdict;
}

// print structures for demo purposes, called after all operations
void printStructs(){
	printf("\n");
	printf("Struct Array is\n\n");
	// print all the content
	for(int i=0;i<INDEXSIZE;i++){
		printf("Content: %s\n",content[i].contentName);
		printf("Peers: \n");
		for(int j=0;j<PEERSIZE;j++){
			// print all the peers at this content
			printf("%s %s %s", content[i].sources[j].peername, content[i].sources[j].port, content[i].sources[j].address);
			if((strcmp(content[i].sources[j].peername,""))){
				printf(" Last Used %d\n", content[i].sources[j].lastUsed);
			}
			else{
				printf("\n");
			}
		}
	printf("\n");
	}
}

// find last used peer by checking lastUsed within struct
int getUnusedPeer(int dex){
	int k=FLAGVERDICT;
	printf("Looking for unused Peer.\n");
	// for all peers at this index
	for (int j = 0; j<PEERSIZE; j++){
		if(content[dex].sources[j].lastUsed == 0){
			// if they are not last used
			if((strcmp(content[dex].sources[j].peername, ""))){
				// if they are not an empty peer slot (a peer exists with lastUsed = 0)
				// set its lastUsed to 1 and return the index of this peer
				content[dex].sources[j].lastUsed = 1;
				k = j;
				j = MAXSIZE;
			}
		}
	}
	// set all other peers as not last used
	for (int j = 0; j<PEERSIZE; j++){
		if (j != k){
			content[dex].sources[j].lastUsed = 0;
		}
	}
	// catch if there is only one peer so we should reuse them even if they have lastUsed = 0
	if(k==FLAGVERDICT){
		for (int j = 0; j<PEERSIZE; j++){	
			if((strcmp(content[dex].sources[j].peername, ""))){
				// if there is one peer that is not empty and they have lastUsed = 1 we should still send their info
				printf("Only one peer present, so resending last used peer.\n");
				content[dex].sources[j].lastUsed = 1;
				k = j;
				j = MAXSIZE;		
			}
		}	
	}
	return k;
}

// check for empty peer list
int checkEmpty(int dex){
	int k = 0;
	// for all peers at this content
	for (int j = 0; j<PEERSIZE; j++){
		if((strcmp(content[dex].sources[j].peername, ""))){
			// set k = 1 if a peer exists (strcmp returns 0 if peer at source[j] = "")
			k = 1;		
			j = MAXSIZE;
		}
	}
	if(k==0){
		// no peers at this content so remove the content from the index
		strcpy(content[dex].contentName ,"");
	}
	return k;
}

// helper function to determine size of file when determining packets to be sent
int fSize(FILE * fp){
	fseek(fp, 0L, SEEK_END);
	int sz = ftell(fp);
	fseek(fp, 0L, SEEK_SET);
	return sz;
}

// not used currently, but cool function that would send the LS terminal command
const char* callLS(){	
	int pid;
	int pipefd[2];
	pipe(pipefd);
	pid = fork();
	if(pid<0){
		exit(-1);
		printf("Fork failed.\n");
	}
	else if (pid == 0){
		close(pipefd[0]);
		dup2(pipefd[1],1);
		close(pipefd[1]);
		execlp ("/bin/ls", "ls", "-la", NULL);
	}
	else{
		char buffer[100000];
		close(pipefd[1]);
		while(read(pipefd[0],buffer,sizeof(buffer))!=0);
		const char* ptr = &buffer[0];
		return ptr;
	}
	
}
