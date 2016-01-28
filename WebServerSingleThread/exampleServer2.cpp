#include <iostream>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <unistd.h>
#include <dirent.h>
#include <vector>
#include "cs360utils.h"


#define STAT_200            " 200 OK\r\n"
#define STAT_404            " 404 Not Found\r\n"
#define F_GIF               "Content-Type: image/gif\r\n"
#define F_HTML              "Content-Type: text/html\r\n"
#define F_JPG               "Content-Type: image/jpg\r\n"
#define F_TXT               "Content-Type: text/plain\r\n"

void parseRequest(int skt) {
	std::vector<char*> headers;
	GetHeaderLines(headers,skt,0);
	std::string requestedResource = "";
	//parse index headers
	for(int i = 0; i < headers.size(); i++) {
		std::cout << headers[i] << std::endl;
		std::string tString(headers[i]);
		int idx;
		//find requested resource in request headers
		if((idx = tString.find("HTTP_GET")) >= 0) {
			std::cout << "FOUND! at index " << idx << std::endl;
			int sidx, eidx;
			sidx = tString.find(" ") + 1;
			eidx = tString.find(" ", sidx);
			requestedResource = tString.substr(sidx, eidx - sidx);
			std::cout << requestedResource << std::endl;
		}
	}
}

void serve(int skt) {
	// read in request headers
	std::vector<char*> headers;
	GetHeaderLines(headers,skt,0);
	std::string requestedResource = "";
	//parse index headers
	for(int i = 0; i < headers.size(); i++) {
		std::cout << headers[i] << std::endl;
		std::string tString(headers[i]);
		int idx;
		//find requested resource in request headers
		if((idx = tString.find("HTTP_GET")) >= 0) {
			std::cout << "FOUND! at index " << idx << std::endl;
			int sidx, eidx;
			sidx = tString.find(" ") + 1;
			eidx = tString.find(" ", sidx);
			requestedResource = tString.substr(sidx, eidx - sidx);
			std::cout << requestedResource << std::endl;
		}
	}

	//check for type of requested resource
	struct stat filestat;
	//check if the file exists
	//file error
	if(stat(requestedResource.c_str(), &filestat)) {
		std::cout << STAT_404 << "\n";
		return;
		//file doesn't exsist
		//write back canned 404 file not found
		//message
	}
	std::cout << "SOMETHING HERE\n";
	//file
	//this is a file
	if(S_ISREG(filestat.st_mode)) {
		cout << requestedResource << " is a regular file \n";
		cout << "file size = " << filestat.st_size << "\n";
		FILE *fp = fopen(requestedResource.c_str(), "r");
		char *buffer = (char *)malloc(filestat.st_size);
		fread(buffer, filestat.st_size, 1, fp);
		printf("Got\n%s", buffer);
		free(buffer);
		fclose(fp);
		std::cout << STAT_200 << "\n";
	}
		//you can use filestat.st_size to fill in
		//the Content-Length header
		//wrtie back a 200 ok
		//then open the file and write its contents to the socket
		//close the socket and it's done
	
	//directory
	if(S_ISDIR(filestat.st_mode)) {
		cout << requestedResource << " is a directory \n";
		DIR *dirp;
		struct dirent *dp;
		
		dirp = opendir(requestedResource.c_str());
		while((dp = readdir(dirp)) != NULL) {
			printf("name %s\n", dp->d_name);
		}
		(void)closedir(dirp);
		std::cout << STAT_200 << "\n";
	}
		//cout is a directory
		//look for index.html
		//if index doesn't exsist
		//get a directory listing
		//serve that as a web page
		//write it all to the socket
		//close the socket and it's done
	//if(close(skt) == SOCKET_ERROR) {
	//	printf("\nCould not close socket\n");
	//	return;
	//}
}

int main (int argc, char* argv[]) {

	// parse input params
	if(argc != 3) {
	std::cout << "There must be three arguments:(\n";
	exit(0);
	}

	int portNumber = atoi(argv[1]);
	std::string rootDir = argv[2];

	//create a socket
	int skt = socket(AF_INET, SOCK_STREAM, 0);
	if(skt == -1) {
		std::cout << "No Socket!\n";
		exit(0);
	}

	//bind socket to port
	struct sockaddr_in servAddr;
	servAddr.sin_family = AF_INET;
	servAddr.sin_port = htons(portNumber);
	servAddr.sin_addr.s_addr = INADDR_ANY;

	int optval = 1;
	setsockopt(skt, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));

	int bindReturn = bind(skt,(sockaddr*) &servAddr, sizeof(servAddr));

	//tell socket to listen
	int listenReturn = listen(skt, 1000);
	int addrSize = sizeof(struct sockaddr_in);
	int i = 0; 
	for(;;) {
	// accept  connection
		struct sockaddr_in clientAddr;
		int connsock = accept(skt, (sockaddr*) &clientAddr, (socklen_t*) &addrSize);
		if(connsock == -1) {
			std::cout << STAT_404 << " time = " << i << "\n";
		}
		else {
			std::cout << STAT_200 << " time = " << i << "\n";
			serve(connsock);
		}
		//std::string msg = "\r\n\r\n<html>hello!</html>\n";
		//int wrtieBytes = write(connsock, msg.c_str(),msg.size());
	}
	//bind socket to port

	return 0;
}
