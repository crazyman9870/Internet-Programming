#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <iostream>
#include <string>
#include <vector>

#define SOCKET_ERROR        -1
#define BUFFER_SIZE         100
#define HOST_NAME_SIZE      255
#define MAX_MSG_SZ          1024

using namespace std;
/* Provided Code in the 360Utils.h file given to us */

// Determine if the character is whitespace
bool isWhitespace(char c)
{
    switch (c)
    {
        case '\r':
        case '\n':
        case ' ':
        case '\0':
            return true;
        default:
            return false;
    }
}

// Strip off whitespace characters from the end of the line
void chomp(char *line)
{
    int len = strlen(line);
    while (isWhitespace(line[len]))
    {
        line[len--] = '\0';
    }
}

// Read the line one character at a time, looking for the CR
// You dont want to read too far, or you will mess up the content
char * GetLine(int fds)
{
    char tline[MAX_MSG_SZ];
    char *line;
    
    int messagesize = 0;
    int amtread = 0;
    while((amtread = read(fds, tline + messagesize, 1)) < MAX_MSG_SZ)
    {
        if (amtread > 0)
            messagesize += amtread;
        else
        {
            perror("Socket Error is:");
            fprintf(stderr, "Read Failed on file descriptor %d messagesize = %d\n", fds, messagesize);
            exit(2);
        }
        //fprintf(stderr,"%d[%c]", messagesize,message[messagesize-1]);
        if (tline[messagesize - 1] == '\n')
            break;
    }
    tline[messagesize] = '\0';
    chomp(tline);
    line = (char *)malloc((strlen(tline) + 1) * sizeof(char));
    strcpy(line, tline);
    //fprintf(stderr, "GetLine: [%s]\n", line);
    return line;
}

// Change to upper case and replace with underlines for CGI scripts
void UpcaseAndReplaceDashWithUnderline(char *str)
{
    int i;
    char *s;
    
    s = str;
    for (i = 0; s[i] != ':'; i++)
    {
        if (s[i] >= 'a' && s[i] <= 'z')
            s[i] = 'A' + (s[i] - 'a');
        
        if (s[i] == '-')
            s[i] = '_';
    }
    
}


// When calling CGI scripts, you will have to convert header strings
// before inserting them into the environment.  This routine does most
// of the conversion
char *FormatHeader(char *str, char *prefix)
{
    char *result = (char *)malloc(strlen(str) + strlen(prefix));
    char* value = strchr(str,':') + 2;
    UpcaseAndReplaceDashWithUnderline(str);
    *(strchr(str,':')) = '\0';
    sprintf(result, "%s%s=%s", prefix, str, value);
    return result;
}

// Get the header lines from a socket
//   envformat = true when getting a request from a web client
//   envformat = false when getting lines from a CGI program

void GetHeaderLines(vector<char *> &headerLines, int skt, bool envformat)
{
    // Read the headers, look for specific ones that may change our responseCode
    char *line;
    char *tline;
    
    tline = GetLine(skt);
    while(strlen(tline) != 0)
    {
        if (strstr(tline, "Content-Length") ||
            strstr(tline, "Content-Type"))
        {
            if (envformat)
                line = FormatHeader(tline, NULL);
            else
                line = strdup(tline);
        }
        else
        {
            if (envformat) 
            {
                string fhead = "HTTP_";
                line = FormatHeader(tline, (char *)fhead.c_str());
            }
            else
            {
                line = (char *)malloc((strlen(tline) + 10) * sizeof(char));
                sprintf(line, "HTTP_%s", tline);
            }
        }
        //fprintf(stderr, "Header --> [%s]\n", line);
        
        headerLines.push_back(line);
        free(tline);
        tline = GetLine(skt);
    }
    free(tline);
}

/* End of Provided Code and begin functional main */ 

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
	string page = argv[optind + 2];
    
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
		
		cout<<"\nConnecting to" << strHostName << " (" << nHostAddress << ") on port "<< nHostPort << "\n";
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
            
			string request = "GET " + page + " HTTP/1.1\r\n"
            + "Host: " + strHostName + "\r\n"
            + "Accept: */*\r\n"
            + "Content-Type: text/html\r\n"
            + "Content-Length: 0\r\n\r\n";
            
            write(hSocket,request.c_str(),request.length());
            
			if(debugMode)
			{
				cout << "\nWriting:\n " << request << "\n to server\n";
			}
			
			
            // This shows how you could use these tools to implement a web client
            // We will talk about how to use them for the server too
            
            int fd;
            vector<char *> headerLines;
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
		cout <<"\nSuccessfull Downloads: " << successfulConnects << "\n\n";
    }
    
    printf("\nClosing socket\n");
    /* close socket */
    if(close(hSocket) == SOCKET_ERROR)
    {
        printf("\nCould not close socket\n");
        return 0;
    }
}