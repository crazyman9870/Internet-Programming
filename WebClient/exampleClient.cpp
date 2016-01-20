#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <string>
#include <vector>

#define SOCKET_ERROR        -1
#define BUFFER_SIZE         100
#define HOST_NAME_SIZE      255

int  main(int argc, char* argv[])
{
	int hSocket;                 /* handle to socket */
	struct hostent* pHostInfo;   /* holds info about a machine */
	struct sockaddr_in Address;  /* Internet socket address stuct */
	long nHostAddress;
	char pBuffer[BUFFER_SIZE];
	unsigned nReadAmount;
	char strHostName[HOST_NAME_SIZE];
	int nHostPort;

	if(argc < 3) {
		printf("\nUsage: client host-name host-port\n");
   		return 0;
	}

	std::string hostname;
	int portNumber;
	std::string res;

	int sockfd =;
	sockfd(AF_INET, SOCK_STREAM, 0);
	//resolve hostname
	struct hostnet* server;
	memset(&server, 0 , sizeof(server));
	server = gethostbyname(hostname.c_str());

	if(server == NULL)
		std::cout << "\nThis didn't work and the srver is null\n";

	//format the address
	struct sockaddr_in serv_addr;
	memset(&serv_addr, 0, sizeof(server));
	memcpy(&serv_addr.sinaddr.s_addr, server->h_addr, server->h_length);

	//connect to the server
//	int connect _status = 

	//properly formatted get request
	std::string request = "GET /CS360/foo.html HTTP/1.0\r\nHost: bioresearch.byu.edu\r\n\r\n"; 

	int writeReturn;
	writeReturn = write(sockfd, request.c_str(), request.size());
	std::cout << "wrote " <<  writeReturn << "bytes\n";

	int readReturn;
	char buff[256];
	memset(buff, 0, sizeof(buff));
	readReturn = read(sockfd, buff, 255);
	std::cout << "read " << readReturn << " bytes\n";



	return 0;
}
