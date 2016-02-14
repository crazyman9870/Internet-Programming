#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <netdb.h>
#include <dirent.h>
#include <fcntl.h>
#include <pthread.h>
#include <semaphore.h>
#include <signal.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <iostream>
#include <vector>
#include <thread>
#include <queue>
#include <fstream>
#include "cs360utils.h"

#define SOCKET_ERROR        -1
#define QUEUE_SIZE          5000
#define MAX_MSG_SIZE		1024
#define DEFAULT_THREADS		20
#define STAT_200            " 200 OK\r\n"
#define STAT_404            " 404 Not Found\r\n"
#define F_GIF               "Content-Type: image/gif\r\n"
#define F_HTML              "Content-Type: text/html\r\n"
#define F_JPG               "Content-Type: image/jpg\r\n"
#define F_TXT               "Content-Type: text/plain\r\n"

//using namespace std;

std::queue<int> priorityQueue;
sem_t mutex; //one person on queue
sem_t full; //work on queue
sem_t empty; //how much is left

void handler(int status);

std::string parseRequest(int skt) {
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
            return requestedResource;
        }
    }
    return STAT_404;
}

struct thread_info
{
    int thread_id;
    int another_number;
};

void* serve(void* in_data) {
    struct thread_info* t_info = (struct thread_info*)in_data;
    int tid = t_info->thread_id;

    while(1)
    {
		fflush(stdout);
		sem_wait(&full);
		fflush(stdout);
		sem_wait(&mutex);
		fflush(stdout);
		if(priorityQueue.size()>0)
		{
			int work_thread = priorityQueue.front();
			priorityQueue.pop();
			fflush(stdout);
		}
		sem_post(&mutex);
		fflush(stdout);
		sem_post(&empty);
    }
}

int main(int argc, char* argv[])
{
    int hSocket,hServerSocket;  /* handle to socket */
    struct hostent* pHostInfo;   /* holds info about a machine */
    struct sockaddr_in Address; /* Internet socket address stuct */
    int nAddressSize=sizeof(struct sockaddr_in);
    int nHostPort, numberOfThreads;
    std::string rootDir = "";


    // parse input params
    if(argc < 3) {
        std::cout << "There must be at least three arguments\n";
        std::cout << "Format ./server portnumber numberofthread(optional) directory\n";
        exit(0);
    }
    if(argc == 3) {
	    nHostPort = atoi(argv[1]);
		rootDir = argv[2];
		numberOfThreads = DEFAULT_THREADS;
    }
    else {
    	nHostPort = atoi(argv[1]);
    	numberOfThreads = atoi(argv[2]);
    	rootDir = argv[3];
    }

    /* Set up the signal handler */
    struct sigaction sigold, signew;
    //signew.sa_handler=handler;
    sigemptyset(&signew.sa_mask);
    sigaddset(&signew.sa_mask,SIGINT);
    signew.sa_flags = SA_RESTART;
    //sigaction(SIGINT, &signew, &sigold);
    //sigaction(SIGHUP, &signew, &sigold);
    sigaction(SIGPIPE, &signew, &sigold);

    struct thread_info threadids[numberOfThreads];
    pthread_t threads[numberOfThreads];
    sem_init(&mutex, PTHREAD_PROCESS_PRIVATE, 1);
    sem_init(&full, PTHREAD_PROCESS_PRIVATE , 0);
    sem_init(&empty, PTHREAD_PROCESS_PRIVATE , 100);

    /* create the correct number of threads */
    for(int i = 0; i < numberOfThreads; i++) {
    	fflush(stdout);
    	sem_wait(&mutex);
    	fflush(stdout);
    	threadids[i].thread_id = i;
    	pthread_create(&threads[i], NULL, serve, (void*)&threadids[i]);
    	sem_post(&mutex);
    	fflush(stdout);
    }

	printf("\nStarting server");
    printf("\nMaking socket");
    /* make a socket */
    hServerSocket=socket(AF_INET,SOCK_STREAM,0);
    if(hServerSocket == SOCKET_ERROR) {
        printf("\nCould not make a socket\n");
        return 0;
    }
    //This allows for the server to reconnect to the same port using reuse address
    int optval = 1;
    setsockopt (hServerSocket, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));
    
    /* fill address struct */
    Address.sin_addr.s_addr=INADDR_ANY;
    Address.sin_port=htons(nHostPort);
    Address.sin_family=AF_INET;
    
    printf("\nBinding to port %d\n",nHostPort);
    
    /* bind to a port */
    if(bind(hServerSocket,(struct sockaddr*)&Address,sizeof(Address)) == SOCKET_ERROR) {
        printf("\nCould not connect to host\n");
        return 0;
    }
    /*  get port number */
    getsockname(hServerSocket, (struct sockaddr *) &Address,(socklen_t *)&nAddressSize);
    printf("opened socket as fd (%d) on port (%d) for stream i/o\n",hServerSocket, ntohs(Address.sin_port));
    
    printf("Server\n\
           sin_family        = %d\n\
           sin_addr.s_addr   = %d\n\
           sin_port          = %d\n"
           , Address.sin_family
           , Address.sin_addr.s_addr
           , ntohs(Address.sin_port)
           );
	fflush(stdout);
 
    printf("\nMaking a listen queue of %d elements",QUEUE_SIZE);
    /* establish listen queue */
    if(listen(hServerSocket,QUEUE_SIZE) == SOCKET_ERROR) {
        printf("\nCould not listen\n");
        return 0;
    }

    int i = 0;
    for(;;) {
    	fflush(stdout);
    	sem_wait(&empty);
    	sem_wait(&mutex);
    	priorityQueue.push(i);
    	sem_post(&mutex);
    	sem_post(&full);
    	fflush(stdout);

        i++;
        char *response = (char *) malloc(sizeof(char) * 1024);
        strcat(response,"HTTP/1.0 ");

        printf("\nWaiting for a connection\n");
        /* get the connected socket */
        hSocket=accept(hServerSocket,(struct sockaddr*)&Address,(socklen_t *)&nAddressSize);

        printf("\nGot a connection from %X (%d)\n", Address.sin_addr.s_addr, ntohs(Address.sin_port));

        //This parses the headers to get the request from the client
        std::string firstLine = parseRequest(hSocket);
        //handles the trailing / on the client side if there is one
        while(firstLine[firstLine.length()-1] == '/') {
        	firstLine = firstLine.substr(0, firstLine.length() - 1);
        }

        char clientPath[36];
        strcpy(clientPath,firstLine.c_str());
        //Check for invalid response
        if (firstLine == STAT_404) {
            std::cout << "There is no GET request\n";
        }

        bool correctPath = false, isDirectory = false;

        struct stat filestat;
        char fullPath[36];
        strcpy(fullPath,rootDir.c_str());
        std::cout << fullPath << " = fullPath 1\n";
        strcat(fullPath,firstLine.c_str());
        std::cout << fullPath << " = fullPath 2\n";
        
        if(stat(fullPath, &filestat) == -1) {
            std::cout <<"\nThe file path does not exsist\n";
        }
        else if(S_ISREG(filestat.st_mode)) {
            char * ext = strrchr(clientPath, '.'); //extension of file path
            if(strcmp(ext,".html") == 0) {
                strcat(response,STAT_200);
                strcat(response,"MIME-Version:1.0\r\n");
                strcat(response,F_HTML);
                char contentLength [50];
                sprintf(contentLength, "Content-Length: %d\r\n\r\n", (int)filestat.st_size);
                strcat(response, contentLength);
                correctPath = true;
            }
            else if(strcmp(ext,".jpg") == 0) {
                strcat(response,STAT_200);
                strcat(response,"MIME-Version:1.0\r\n");
                strcat(response,F_JPG);
                char contentLength [50];
                sprintf(contentLength, "Content-Length: %d\r\n\r\n", (int)filestat.st_size);
                strcat(response, contentLength);
                correctPath = true;
            }
            else if(strcmp(ext,".gif") == 0) {
                strcat(response,STAT_200);
                strcat(response,"MIME-Version:1.0\r\n");
                strcat(response,F_GIF);
                char contentLength [50];
                sprintf(contentLength, "Content-Length: %d\r\n\r\n", (int)filestat.st_size);
                strcat(response, contentLength);
                correctPath = true;
            }
            else if(strcmp(ext,".txt") == 0) {
                strcat(response,STAT_200);
                strcat(response,"MIME-Version:1.0\r\n");
                strcat(response,F_TXT);
                char contentLength [50];
                sprintf(contentLength, "Content-Length: %d\r\n\r\n", (int)filestat.st_size);
                strcat(response, contentLength);
                correctPath = true;
            }
        }
        // After determining if there is a directory we need to determine if there exists a index.html
        else if(S_ISDIR(filestat.st_mode)) {
            correctPath = true;
            struct stat indexFile;
            char indexPath[36];
            strcpy(indexPath,fullPath);
            strcat(indexPath,"/index.html");

            // If there is no index.html we flag as a directory and unflag the correct path
            if(stat(indexPath, &indexFile)) {
                isDirectory = true;
                correctPath = false;
                /*strcat(response,STAT_200);
                strcat(response,"MIME-Version:1.0\r\n");
                strcat(response,F_HTML);
                char contentLength [50];
                sprintf(contentLength, "Content-Length: %d\r\n\r\n", (int)indexFile.st_size);
                strcat(response, contentLength);*/
                strcpy(fullPath,indexPath);
            }
            // Else we have the correct path and we will load the .html that is associated with the page
            else {
                strcat(response,STAT_200);
                strcat(response,"MIME-Version:1.0\r\n");
                strcat(response,F_HTML);
                char contentLength [50];
                sprintf(contentLength, "Content-Length: %d\r\n\r\n", (int)indexFile.st_size);
                strcat(response, contentLength);
                strcpy(fullPath,indexPath);
            }
        }

        if(correctPath) {

            FILE *file = fopen(fullPath,"r");
            write(hSocket, response, strlen(response));
            bzero((char*) &response, sizeof(response));

            // Write the file
            //This will send bytes in chunks of 256 and will continue until it hits the end of the file
            for(;;) {

                unsigned char buffer[256] = {0}; 
                int bytesRead = fread(buffer, 1, 256, file);
                if(bytesRead > 0) {
                    write(hSocket, buffer, bytesRead);
                }
                //This flag will catch the read when it gets a buffer that is not entirely filled
                //After wrting those last bytes the \r\n\r\n tag will be in the file to allow the loop to terminate
                if(bytesRead < 256) {
                    if(feof(file)) {
                        std::cout << "Has readched the end of the file\n";
                    }
                    if(ferror(file)) {
                        std:cout << "There was an error reading the file to the socket\n";
                    }
                    break; //Get us out of the infinite for loop
                }
            }
        }
        //Need to generate an html for the directory listing
        else if(isDirectory) {
                std::cout << "WE MADE IT TO THE DIRECTORY\n";
                std::vector<std::string> subdirList;
                DIR *dir;
                struct dirent *dirEntries;
                std::string directoryPath = fullPath;

                if((dir = opendir(directoryPath.substr(0, directoryPath.length() - 11).c_str())) != NULL) {
                    while((dirEntries = readdir(dir)) != NULL) {
                        subdirList.push_back((std::string) dirEntries->d_name);
                    }
                    closedir(dir);
                }
                std::string html = "<!DOCTYPE html><html><head><title> Directory listing for ";
                html += clientPath;
                html += "</title></head><body>";
                html += "<h1> Directory for ";
                html += clientPath;
                html += "</h1><div>";

                std::string relativePath = clientPath;
                //std::cout << relativePath << " = relativePath after cut\n";
                for(int i = 0; i < subdirList.size(); i++) {
                	if(subdirList[i] == ".." || subdirList[i] == ".") {
                		continue;
                	}
                    html += "<li><a href=\"";
                    html += relativePath.substr(1, relativePath.length() - 1);
                    html += "/";
                    html += subdirList[i];
                    html += "\">";
                    html += subdirList[i];
                    html += "</a></li>";
                }

                html += "</div></body></html>";

                strcat(response,STAT_200);
                strcat(response,"MIME-Version:1.0\r\n");
                strcat(response,F_HTML);
                char contentLength [50];
                sprintf(contentLength, "Content-Length: %d\r\n\r\n", (int)html.length());
                strcat(response, contentLength);

                //std::cout << html << std::endl;

                write(hSocket, response, strlen(response));
                write(hSocket, html.c_str(), html.length());
                bzero((char*) &response, sizeof(response));

        }
        else {

            std::string errorHTML = "<html><head><title>404 - Not Found</title></head>";
            errorHTML += "<body><h1>404 - Not Found</h1></body></html>";

            strcat(response,STAT_404);
            strcat(response,F_HTML);
            char contentLength [50];
            sprintf(contentLength, "Content-Length: %d\r\n\r\n", (int)errorHTML.length());
            strcat(response, contentLength);

            write(hSocket, response, strlen(response));
            bzero((char*) &response, sizeof(response));         
            write(hSocket, errorHTML.c_str(), errorHTML.length());
        }

        //shutdown(hSocket, SHUT_RDWR);
        /* close socket */
        printf("\nClosing the socket");
        if(close(hSocket) == SOCKET_ERROR)
        {
         printf("\nCould not close socket\n");
         return 0;
        }
    }
}
