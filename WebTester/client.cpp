#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <iostream>
#include <string>
#include <vector>
#include "cs360utils.h"

#define SOCKET_ERROR        -1
#define HOST_NAME_SIZE      255
#define MAX_MSG_SZ          1024

int main(int argc, char *argv[])
{
    int hSocket;                 /* handle to socket */
    struct hostent* pHostInfo;   /* holds info about a machine */
    struct sockaddr_in Address;  /* Internet socket address stuct */
    long nHostAddress;
    char pBuffer[BUFFER_SIZE];
    unsigned nReadAmount;
    char strHostName[HOST_NAME_SIZE];
    int nHostPort;
    int downloadAttempts = 1;
    int c = 0;
    bool debugMode = false;
    bool countMode = false;
    extern char *optarg;
    
    //Check to make sure we have the minimum number of arguments
    if(argc < 3)
    {
        printf("\nUsage: client host-name host-port\n");
        return 0;
    }
   
    //Provided code
    //Says look for a c followed with an argument for countMode or a d for debugMode
    while((c = getopt(argc, argv, "c:d")) != -1)
    {
        switch(c)
        {
            case 'c':
                countMode = true;
                downloadAttempts = atoi(optarg);
                break;
            case 'd':
                debugMode = true;
                break;
            case '?':
                break;
        }
    }
    
	//Grab the Host, Port, and the page to connect to
	strcpy(strHostName,argv[optind]);
	nHostPort = atoi(argv[optind + 1]);
	std::string page = argv[optind + 2];
    
    printf("\nMaking a socket");
    /* make a socket */
    
    int successfulConnects = 0;
    for(int attempt = 0; attempt < downloadAttempts; attempt++)
    {
        hSocket=socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
        if(hSocket == SOCKET_ERROR)
        {
            printf("\nCould not make a socket\n");
            return 0;
        }
        
        /* get IP address from name */
        pHostInfo=gethostbyname(strHostName);
        if(pHostInfo==NULL)
        {
            printf("\nERROR: no such hostname\n");
            return 0;
        }
        
        /* copy address into long */
        memcpy(&nHostAddress,pHostInfo->h_addr,pHostInfo->h_length);
        
        /* fill address struct */
		bzero((char*) &Address, sizeof(Address));
		bcopy((char*) pHostInfo->h_addr, (char*) &Address.sin_addr.s_addr, pHostInfo->h_length);
//        Address.sin_addr.s_addr=nHostAddress;
        Address.sin_port=htons(nHostPort);
        Address.sin_family=AF_INET;
		
		std::cout<<"\nConnecting to" << strHostName << " (" << nHostAddress << ") on port "<< nHostPort << "\n";
//        printf("\nConnecting to %s (%X) on port %d",strHostName,nHostAddress,nHostPort);
        /* connect to host */
        if(connect(hSocket,(struct sockaddr*)&Address,sizeof(Address)) == SOCKET_ERROR)
        {
            printf("\nCould not connect to host\n");
            if(!countMode)
            {
                return 0;
            }
        }
        else
        {
            successfulConnects++;
            
            /* read from socket into buffer
             ** number returned by read() and write() is the number of bytes
             ** read or written, with -1 being that an error occured */
            
			std::string request = "GET " + page + " HTTP/1.1\r\n"
            + "Host: " + strHostName + "\r\n"
            + "Accept: */*\r\n"
            + "Content-Type: text/html\r\n"
            + "Content-Length: 0\r\n\r\n";
            
            write(hSocket,request.c_str(),request.length());
            
			if(debugMode)
			{
				std::cout << "\nWriting:\n " << request << "\n to server\n";
			}
			
			
            // This shows how you could use these tools to implement a web client
            // We will talk about how to use them for the server too
            
            int fd;
            std::vector<char *> headerLines;
            char buffer[MAX_MSG_SZ];
            char contentType[MAX_MSG_SZ];
            
            fd = hSocket;
            if(fd < 0)
            {
                perror("open of sample.txt failed");
                exit(0);
            }
            
            // First read the status line
            char *startline = GetLine(fd);
            if(debugMode)
            {
                printf("Status line %s\n\n",startline);
            }
            
            // Read the header lines
            GetHeaderLines(headerLines, fd , false);
            
            
            // Now print them out
            for (int i = 0; i < headerLines.size(); i++)
            {
                if(debugMode)
                {
                    printf("%s\n",headerLines[i]);
                }
                if(strstr(headerLines[i], "Content-Type"))
                {
                    sscanf(headerLines[i], "Content-Type: %s", contentType);
                }
            }
            
            if(debugMode)
            {
                printf("\n=======================\n");
                printf("Headers are finished, now read the file\n");
                printf("Content Type is %s\n",contentType);
                printf("=======================\n\n");
            }
            
            // Now read and print the rest of the file
            int rval;
            while((rval = read(fd,buffer,MAX_MSG_SZ)) > 0)
            {
                if(!countMode)
                {
                    write(1,buffer,rval);
                }
                break;
            }
        }
    }
    
    if(countMode)
    {
		std::cout <<"\nSuccessfull Downloads: " << successfulConnects << "\n\n";
    }
    
    printf("\nClosing socket\n");
    /* close socket */
    if(close(hSocket) == SOCKET_ERROR)
    {
        printf("\nCould not close socket\n");
        return 0;
    }
}
