#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <netdb.h>
#include <math.h>
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
#define NSTD 3

int main(int argc, char *argv[]) {
	
    struct hostent* pHostInfo;   /* holds info about a machine */
    struct sockaddr_in Address;  /* Internet socket address stuct */
    long nHostAddress;
    char pBuffer[BUFFER_SIZE];
    unsigned nReadAmount;
    char strHostName[HOST_NAME_SIZE];
    int nHostPort;
    int downloadAttempts = 1;
    bool debugFlag = false;
    int c = 0;
    extern char *optarg;

	if(argc < 4) {
        printf("\nUsage: webtest host-name host-port testnumber\n");
        printf("You can also add the -d falg before or after testnumber\n");
        return 0;
	}

	// Set the counting flag and then the number of sockets
    while((c = getopt(argc, argv, "d")) != -1)
    {
        switch(c)
        {
            case 'd':
                //downloadAttempts = atoi(optarg);
            	debugFlag = true;
                break;
            case '?':
                break;
        }
    }

	//Grab the Host, Port, and the page to connect to
	strcpy(strHostName,argv[optind]);
	nHostPort = atoi(argv[optind + 1]);
	downloadAttempts = atoi(argv[optind + 2]);
	std::cout << "downloadAttempts = " << downloadAttempts << std::endl;
	if(debugFlag)
		std::cout << "TRUEMUTHATRUCKA\n";


    //Creat the list of times
	int hSocket[downloadAttempts+NSTD];				/* handle to socket */ 
	struct timeval oldtime[downloadAttempts];
	double recordedTimes[downloadAttempts];
	double totalTime = 0;
	double timeMax = 0;
	double timeMin = 100000;

    /* make a socket */
	printf("\nMaking a socket\n");
    /* get IP address from name */
	for(int i = 0; i < downloadAttempts; i++) {
	    hSocket[i]=socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);

		if(hSocket[i] == SOCKET_ERROR)
		{
			printf("\nCould not make a socket\n");
			return 0;
		}
	}

    /* get IP address from name */
    pHostInfo=gethostbyname(strHostName);
    /* copy address into long */
    memcpy(&nHostAddress,pHostInfo->h_addr,pHostInfo->h_length);

    /* fill address struct */
    Address.sin_addr.s_addr=nHostAddress;
    Address.sin_port=htons(nHostPort);
    Address.sin_family=AF_INET;

	int epollFD = epoll_create(1);
	// Send the requests and set up the epoll data
	for(int i = 0; i < downloadAttempts; i++) {
		/* connect to host */
		if(connect(hSocket[i],(struct sockaddr*)&Address,sizeof(Address)) == SOCKET_ERROR) {
			printf("\nCould not connect to host\n");
			return 0;
		}
		char request[] = "GET /test.html HTTP/1.0\r\n\r\n";

	    write(hSocket[i],request,strlen(request));
		// Keep track of the time when we sent the request
		gettimeofday(&oldtime[hSocket[i]],NULL);
		// Tell epoll that we want to know when this socket has data
		struct epoll_event event;
		event.data.fd = hSocket[i];
		event.events = EPOLLIN;
		int ret = epoll_ctl(epollFD,EPOLL_CTL_ADD,hSocket[i],&event);
		if(ret)
			perror ("epoll_ctl");
	}
	for(int i = 0; i < downloadAttempts; i++) {
		struct epoll_event event;
		int rval = epoll_wait(epollFD,&event,1,-1);
		if(rval < 0)
			perror("epoll_wait");
		rval = read(event.data.fd,pBuffer,BUFFER_SIZE);
		struct timeval newtime;
		// Get the current time and subtract the starting time for this request.
		gettimeofday(&newtime,NULL);
		double sec = ((newtime.tv_sec - oldtime[event.data.fd].tv_sec)*(double)1000000+(newtime.tv_usec-oldtime[event.data.fd].tv_usec))/1000000;
		/*Calulate the total amount of time for the average */
		recordedTimes[i] = sec;
		totalTime += sec;
		/* Sets the max and min time to calulate the standard deviation */ 
		if(timeMax < sec)
			timeMax = sec;
		if(timeMin > sec)
			timeMin = sec;
		if(debugFlag) {
			std::cout << "Response " << i+1 << " time = " << sec << std::endl;
		}
		//std::cout << "Time " << sec;
		//std::cout << "new time tv_sec = " << newtime.tv_sec << "\n";
		//std::cout << "new time tv_usec = " << newtime.tv_usec << "\n";
		//std::cout << "old time tv_sec = " << oldtime[event.data.fd-3].tv_sec << "\n";
		//std::cout << "old time tv_usec = " << oldtime[event.data.fd-3].tv_usec << "\n";
		//printf(" got %d from request %d\n",rval,event.data.fd-2);
		// Take this one out of epoll
		epoll_ctl(epollFD,EPOLL_CTL_DEL,event.data.fd,&event);
	}

	double mean = totalTime/downloadAttempts;
	double variance = 0; 
	for(int i = 0; i < downloadAttempts; i++) {
		variance += pow((recordedTimes[i] - mean), 2.0);
	}
	variance = variance/downloadAttempts;
	double standardDeviation = sqrt(variance);

	std::cout << "\nConnections Made = " << downloadAttempts << "\n";
	std::cout << "Max = " << timeMax << "\n";
	std::cout << "Min = " << timeMin << "\n";
	std::cout << "Average Time = " << totalTime/downloadAttempts << "\n";
	std::cout << "Standard Deviation = " << standardDeviation << "\n";


	// Now close the sockets
	for(int i = 0; i < downloadAttempts+NSTD; i++) {
		printf("\nClosing socket\n");
		/* close socket */                       
		if(close(hSocket[i]) == SOCKET_ERROR)
		{
			printf("\nCould not close socket\n");
			return 0;
		}
	}
	return 0;
}