#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h> 
#include <unistd.h> 
#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <string.h>

int connectpeer(char * address,int portno,char * filename) 
{
   int sockfd, n;
   struct sockaddr_in serv_addr;
   struct hostent *server;
   struct in_addr ipv4addr;
   char buffer[256];
   sockfd = socket(AF_INET, SOCK_STREAM, 0);
   if (sockfd < 0) herror("ERROR opening socket");
   inet_pton(AF_INET, address, &ipv4addr);
   server = gethostbyaddr(&ipv4addr, sizeof ipv4addr, AF_INET);
   if (server == NULL) 
   {
      fprintf(stderr,"ERROR, no such host\n");
      exit(0);
   }
   bzero((char *) &serv_addr, sizeof(serv_addr));
   serv_addr.sin_family = AF_INET;
   bcopy((char *)server->h_addr_list[0], (char *)&serv_addr.sin_addr.s_addr, server->h_length);
   serv_addr.sin_port = htons(portno);
   if (connect(sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) //try to connect to the server
   {
      perror("ERROR connecting");
      exit(1);
   }
   printf("Connection to the Peer SUCCESSFUL.\nSending File transfer Request message with the file name....\n");
   char req[50]; //ask msg from user to be read by server
   char* buff="REQUEST : FILE :";
	sprintf(req,"%s %s",buff, filename);
   n = write(sockfd, req, strlen(req)); //send msg to server
   if (n < 0) 
   {
      perror("ERROR writing to socket");
      exit(1);
   }
   bzero(buffer,256);
   n = read(sockfd, buffer, 255); //read server response
   if (n < 0) 
   {
      perror("ERROR reading from socket");
      exit(1);
   }
   printf("received the reply :-%s\n",buffer);
   if(strcmp(buffer,"File NOT FOUND")==0)
   {
	   printf("Closing the connection gracefully since file NOT FOUND on this node...\n");
	   n=shutdown(sockfd,0); //close the connection gracefully since file not found
	   if (n < 0) 
      {
		perror("ERROR closing the connection");
		exit(1);
		}
	}
	else  if(strcmp(buffer,"File FOUND")==0)
   {
		printf("FOUND the file...\n");
		n = read(sockfd, buffer, 255);//read the file content the peer is sending
		if (n < 0) 
      {
			perror("ERROR reading from socket");
			exit(1);
      }
		printf("File has the following content - \n%s",buffer);
		printf("gracefully closing the connection with the peer....\n");
		n=shutdown(sockfd,0);
		if (n < 0) 
      {
			perror("ERROR closing the connection");
			exit(1);
		}
		return 0;
	}
	else printf("received unknown reply from the node\n");
   return -1;
}

int getFile(int sockfd)
{
	char* req="REQUEST : peer info",buffer[256]; //request for active peer information
	int n;
   n = write(sockfd, req, strlen(req)); //send msg to server
   if (n < 0) 
   {
      perror("ERROR writing to socket");
      exit(1);
   }
   bzero(buffer,256);
   n = read(sockfd, buffer, 255); //read server response
   if (n < 0) 
   {
      perror("ERROR reading from socket");
      exit(1);
   }
   printf("Receive the following response - \n%s\n",buffer);
   printf("gracefully closing the connection with the relay server....\n");
   n=shutdown(sockfd,0);
	if (n < 0) {
	perror("ERROR closing the connection");
	exit(1);
	}
	FILE * peers=fopen("peer.txt","w"); //store the info in a file
	fprintf(peers,"%s",buffer);
	fclose(peers); 
   char  file[50];
   printf("Enter the File name : ");
   scanf("%s",file);
   char peerName[INET_ADDRSTRLEN];
   int port,flag=0;
   peers=fopen("peer.txt","r");
   while(fscanf(peers,"%s %d",peerName,&port)!=EOF) //process the response one peer at a time and try to fetch the file
   {
	   printf("Connecting to the peer %s:%d...\n",peerName,port);
	   n = connectpeer(peerName,port,file);
	   if(n<0) continue;
	   else //successfult found the file on this node
      {
         flag=1;
         break;
      }
   }
   fclose(peers);
   if(!flag) printf("File not found on any node!\n");
	return 0;
}

int main(int argc, char *argv[]) 
{
   int sockfd, portno, n;
   struct sockaddr_in serv_addr;
   struct hostent *server;
   char buffer[256];
   if (argc < 3) 
   {
      fprintf(stderr,"usage %s hostname port\n", argv[0]);
      exit(0);
   }
   portno = atoi(argv[2]);
   sockfd = socket(AF_INET, SOCK_STREAM, 0); //create a socket point
   if (sockfd < 0) 
   {
      perror("ERROR opening socket");
      exit(1);
   }
   server = gethostbyname(argv[1]);
   if (server == NULL) 
   {
      fprintf(stderr,"ERROR, no such host\n");
      exit(0);
   }
   bzero((char *) &serv_addr, sizeof(serv_addr));
   serv_addr.sin_family = AF_INET;
   bcopy((char *)server->h_addr_list[0], (char *)&serv_addr.sin_addr.s_addr, server->h_length);
   serv_addr.sin_port = htons(portno);
   if (connect(sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0)  //connect to server
   {
      perror("ERROR connecting");
      exit(1);
   }
   printf("Connecting to the relay server. Sending Request message\n"); //ask msg from user to be read by server
   char* req="REQUEST : client";
   n = write(sockfd, req, strlen(req)); //send msg to server
   if (n < 0) 
   {
      perror("ERROR writing to socket");
      exit(1);
   }
   bzero(buffer,256);
   n = read(sockfd, buffer, 255); //read server response
   if (n < 0) 
   {
      perror("ERROR reading from socket");
      exit(1);
   }
   printf("%s\n",buffer);
   if(buffer[19]=='1')
   {
	   printf("RESPONSE : client accepted\nSUCESSFULLY connected\nFetcing peer info\n");
	   n = getFile(sockfd); //start server if node accepted by relay
		
      if (n < 0) 
      {
         perror("ERROR getting the requested file from the peers");
         exit(1);
      }
   }
   else printf("Node not accepted by the relay server, try again..\n");
   return 0;
}
