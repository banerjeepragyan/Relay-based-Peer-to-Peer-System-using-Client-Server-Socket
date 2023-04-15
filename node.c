#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h> 
#include <unistd.h> 
#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <string.h>

int startserver(char * port)
{
   int sockfd, newsockfd,portno;
   socklen_t clilen;
   char buffer[256];
   struct sockaddr_in serv_addr, cli_addr;
   int n;
   sockfd = socket(AF_INET, SOCK_STREAM, 0);
   if (sockfd < 0) herror("ERROR opening socket");
   bzero((char *) &serv_addr, sizeof(serv_addr));
   portno = atoi(port); 
   serv_addr.sin_family = AF_INET;
   serv_addr.sin_addr.s_addr = INADDR_ANY;
   serv_addr.sin_port = htons(portno);
   if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) herror("ERROR on binding");
   listen(sockfd,5);
   printf("Server running on peer node, listening.....\n");
   clilen = sizeof(cli_addr);
   newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
   if (newsockfd < 0) herror("ERROR on accept");
   close(sockfd);
   bzero(buffer,256);
   n = read(newsockfd,buffer,255);
   if (n < 0) herror("ERROR reading from socket");
   printf("Got a message from peer, Here is the message: %s\n",buffer);
   char a[]="REQUEST : FILE : ";
   int i,flag=0;
   for(i=0;i<strlen(a);i++) //check if the message is a request for file
   {
		if(a[i]==buffer[i]) flag=1; 
		else 
      {
			flag=0;
			break;
		}
	}
   if(flag)
   {
      printf("received request for the file : %s\n",&buffer[strlen(a)]);
      FILE * file=fopen(&buffer[strlen(a)],"r");
      if(file==NULL) //if file not found
      {
         printf("requested file NOT Found\n");
         char response[]="File NOT FOUND";
         n = write(newsockfd,response,strlen(response));
         if (n < 0) herror("ERROR writing to socket");
      }
      else 
      {
         printf("Found the requested file \n");
         char response[]="File FOUND";
         n = write(newsockfd,response,strlen(response));
         if (n < 0) herror("ERROR writing to socket");
         fseek(file, 0, SEEK_END); //send the file
         long fsize = ftell(file);
         fseek(file, 0, SEEK_SET); //send the pointer to beginning of the file
         char *string = malloc(fsize + 1);
         fread(string, fsize, 1, file);
         fclose(file);
         printf("file has the following content:\n%s",string);
         n = write(newsockfd,string,strlen(string)); //send the content to client
         if (n < 0) 
         {
            perror("ERROR writing to socket");
            exit(1);
         }
      }
	}
	else 
   {
		printf("received request is not of file name,closing connection......\n");
		close(newsockfd);
	} 
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
   sockfd = socket(AF_INET, SOCK_STREAM, 0); 
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
   if (connect(sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) 
   {
      perror("ERROR connecting");
      exit(1);
   }
   printf("Connecting to the relay server.Sending Request message\n"); //ask msg from user to be read by server
   char* req="REQUEST : node";
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
   
   if(buffer[17]=='1') //start server if node accepted by relay
   {
	   printf("RESPONSE : Node accepted\nSUCESSFULLY connected\n");
	   printf("Closing the connection gracefully...\n");
	   n=shutdown(sockfd,0); //close the connection gracefully
	   if (n < 0) 
      {
         perror("ERROR closing the connection");
         exit(1);
		}
	   printf("port no. of the client for listening is : %s\n",&buffer[20]);
	   startserver(&buffer[20]); //start the server
   }
   else printf("Node not accepted by the relay server, try again..\n");
   return 0;
}
