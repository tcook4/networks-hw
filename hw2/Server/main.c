/* Written by Tyler Cook
 * UNT CSCE 3530
 * Homework 2
 * February 28th, 2018
 * Description:
 */

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>


int connectWebServer(char* website);

void sendFile(char* filename, int clientSocket);

void sendMessage(const char *message, int clientSocket);


int main(int argc, char **argv)
{
    char buffer[1024];                  // Communication buffer between client and server
    int listenFd, clientFd, n;          // File descriptors and error checking
    struct sockaddr_in servaddr;        // Server address structure
    int netDataLength, hostDataLength;  // Length of messages in network / host order
    int portNumber;                     // Port number to use if supplied
    int connected, cached;              // Bools for connection and cached status
    int bytesRead;                      // Number of bytes read in a fread() operation
    char readBuf[512];                  // Read buffer for list.txt file IO
    FILE *fp;                           // File pointer for web data , list.txt and allowlist.txt
    int webSockFd;                      // File descriptor for our web server
    char response[100];                 // Storage for first line of http response
    char fmtAddr[100];                  // Formatted domain name sotrage
    char address[1024];                 // Storage for address to search
    char* searchPtr;                    // Search ptr for http detection
    time_t rawtime;                     // Time info
    struct tm *timeinfo;                // Time struct
    char timeText[16];                  // Storage for time string
    char *subRule, *domainRule, *topRule;
    char* checkSub, *checkDomain, *checkTop;
    char* tempstr; // used in strtok url splitting
    char *wildcard = "*"; // allowlist wildcard
    char *compareWild; // assigned to allowlist
    int allowed1, allowed2, allowed3;



    // Set our http GET message
    char* message = "GET /\r\n HTTP /1.1.";

    // Not connected to a clientyet
    connected = 0;

    /*
    // Verify we have correct number of arguments
    if (argc != 2)
    {
        printf("Error: Program usage: %s port_number\n", argv[0]);
        exit(1);
    }
    else
    {
        portNumber = atoi(argv[1]);
    }
    */

    portNumber = 8857;


    // Connect to client
    listenFd = socket(AF_INET, SOCK_STREAM, 0);
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htons(INADDR_ANY);
    servaddr.sin_port = htons(portNumber);

    // Binds the above details to the socket
    bind(listenFd, (struct sockaddr *) &servaddr, sizeof(servaddr));

    // Start listening to incoming connections
    listen(listenFd, 10);
    printf("Listening for connection...\n");

    // Accept an incoming connection
    clientFd = accept(listenFd, (struct sockaddr*)NULL, NULL);
    if (clientFd < 0) // verify we accepted correctly
    {
        perror("ERROR on accept");
        exit(1);
    }
    printf("Successfully connected to client!\n");
    connected = 1;

    while (connected)
    {
        // Initialization
        bzero(address, 1024);
        bzero(fmtAddr, 100);
        bzero(buffer, 1024);
        netDataLength = 0;
        cached = 0;

        // Get message size and convert from network order
        n = read(clientFd, (char*)&netDataLength, sizeof(netDataLength));
        if (n < 0)
        {
            perror("Error getting data size\n");
        }
        hostDataLength = ntohl(netDataLength);

        // Read the website address
        bzero(buffer, 1024);
        n = read(clientFd, buffer, hostDataLength);
        if (n < 0)
        {
            perror("Error reading message from client\n");
        }
        strcpy(address, buffer);

        // Remove http(s) from address for gethostbyname to work
        if ((searchPtr = strstr(address, "http://")))
        {
            strncpy(fmtAddr, &address[7], 100);
        }
        else if ((searchPtr = strstr(address, "https://")))
        {
            strncpy(fmtAddr, &address[8], 100);
        }
        else // If we didn't find either
        {
            strncpy(fmtAddr, address, 100);
        }

        // Check allow list to ensure we can access this website
        bzero(address, sizeof(address));
        fp = fopen("allowlist.txt", "r");
        if (fp == NULL)
        {
            perror("Error: can not find allowlist.txt in current directory\n");
        }
        else
        {
            // Copy the formatted address so we can strtok it
            tempstr = calloc(strlen(fmtAddr)+1, sizeof(char));
            strcpy(tempstr, fmtAddr);

            // Split our url on "."
            checkSub = strtok(tempstr, ".");
            checkDomain = strtok(NULL, ".");
            checkTop = strtok(NULL, ".");

            // Check allowlist for website we're trying to access
            while(fgets(address, sizeof(address), fp) != NULL)
            {
                subRule = strtok(address, ".");
                if (subRule == NULL)
                {
                    printf("breaking on subrule null\n");
                    break;
                }
                domainRule = strtok(NULL, ".");
                if (domainRule == NULL)

                {
                    printf("breaking on domainrule null\n");
                    break;
                }
                topRule = strtok(NULL, ".");
                if (topRule == NULL)

                {
                    printf("breaking on toprule null\n");
                    break;
                }


                printf("Sub, domain and top are %s %s %s\n", subRule, domainRule, topRule);

                // Check if our subdomain is allowed
                compareWild = &subRule[7];
                printf("Wild compare is %s\n", compareWild);
                if ((strstr(subRule, checkSub)) || ((strcmp(wildcard, compareWild)) == 0))
                {
                    printf("url allowed on subdomain rule\n");
                    allowed1 = 1;
                }

                compareWild = &domainRule[0];
                printf("Wild compare is %s\n", compareWild);
                if ((strstr(domainRule, checkDomain)) || (strcmp(wildcard, compareWild) == 0))
                {
                    printf("url allowed on domain rule\n");
                    allowed2 = 1;
                }

                compareWild = &topRule[0];
                printf("Wild compare is %s\n", compareWild);
                if ((strstr(topRule, checkTop)) || (strcmp(wildcard, compareWild) == 0))
                {
                    printf("url allowed on top rule\n");
                    allowed3 = 1;
                }
            }
            free(tempstr);
        }

        // Check cache for cached version
        bzero(readBuf, 512);
        fp = fopen("list.txt", "r");
        if (fp != NULL)
        {
            while (fgets(readBuf, sizeof(readBuf)-1, fp))
            {
                if(strstr(readBuf, fmtAddr))
                {
                    printf("Found %s in cache, fowarding to user...\n", fmtAddr);
                    sendFile(fmtAddr, clientFd);
                    cached = 1;
                    break;
                }
            }
            fclose(fp);
        }

        if (!cached)
        {
            // Connect to the webpage
            webSockFd = connectWebServer(fmtAddr);
            if (webSockFd == -1)
            {
                printf("Connection failed: website not found\n\n");
                sendMessage("Website not found, please try again\n", clientFd);
                continue;
            }

            // Send GET request to web server
            n = write(webSockFd, message, sizeof(message));
            if (n < 0)
            {
                perror("Error writing to website...\n");
                continue;
            }

            // Open the file in preparation to recieve data
            fp = fopen(fmtAddr, "w+");
            if (fp == NULL)
            {
                perror("Error opening file\n");
                continue;
            }

            // Read response from web server
            bytesRead = 0;
            do
            {
                // Zero our buffer, read from the web page, and store buffer in our file
                bzero(buffer, sizeof(buffer));
                bytesRead = read(webSockFd, buffer, sizeof(buffer));
                fprintf(fp, "%s", buffer);
            }
            while (bytesRead > 0);

            // Rewrind to beginning of file and grab first line
            bzero(response, 100);
            fseek(fp, 0, SEEK_SET);
            fgets(response, 100, fp);
            fclose(fp);

            // If response code is 200, send page to client
            if (strstr(response, "200 OK"))
            {
                printf("Response code 200 - sending webpage to client\n");
                sendFile(fmtAddr, clientFd);

                // Get the time and append it to our address string
                time(&rawtime);
                timeinfo = localtime(&rawtime);
                strftime(timeText, sizeof(timeText)-1, "%Y%m%d%H%M%S", timeinfo);
                timeText[14] = 0;
                strcat(fmtAddr, " ");
                strcat(fmtAddr, timeText);
                strcat(fmtAddr, "\n");

                // Append to list.txt
                fp = fopen("list.txt", "a");
                if (fp == NULL)
                {
                    perror("Error opening list.txt\n");
                }
                else
                {
                    fprintf(fp, "%s", fmtAddr);
                    fclose(fp);
                }
            }
            // If response not 200, send the response and delete cached webpage
            else
            {
                printf("Response code not 200... response: %s\n", response);

                // Send response to client
                sendMessage(response, clientFd);

                // Remove cached version
                remove(fmtAddr);
            }
        }
    }

    close(clientFd);
    return 0;
}


// Connect to an external web server
// This function attempts to connect to the website hostname given
// Returns a socket point if successful, -1 otherwise
int connectWebServer(char *website)
{
    int sockFd;                          // Our socket point
    struct sockaddr_in serv_addr;        // Socket struct
    struct hostent *server;              // Socket host struct


    printf("Connecting to %s...\n", website);

    // Create a socket point
    sockFd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockFd < 0)
    {
        perror("ERROR opening socket");
        return -1;
    }

    // Assign and address our server
    server = gethostbyname(website);
    if (server == NULL)
    {
        fprintf(stderr,"ERROR, no such host\n");
        return -1;
    }

    // Fill in our server address struct
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr, server->h_length);
    serv_addr.sin_port = htons(80); // Connect on port 80

    // Connect to our server
    if (connect(sockFd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0)
    {
        perror("ERROR connecting");
        return -1;
    }

    printf("Successfully connected to web server.\n");
    return sockFd;
}


// Send a file to a client socket connection
// This function is used to send a cached HTTP webpage to a client
// Accepts a filename and socket point, and does not return
void sendFile(char *filename, int clientSocket)
{
    FILE *fp;                       // File pointer
    char buffer[1024];              // Buffer used to store and send file information
    uint32_t networkFileSize;       // Network order filesize
    int fileSize;                   // Length of the file we're sending
    int sent;                       // Bits sent in a network write() operation
    int bytesRead;                  // Number of bytes read from our file
    int offset;                     // Used to keep track of our file position


    printf("Sending file to client...\n");

    // Open file
    fp = fopen(filename, "r");
    if (fp == NULL)
    {
        perror("File open failed: ");
        return;
    }

    // Seek to the end and find file length
    fseek(fp, 0, SEEK_END);
    fileSize = (int)ftell(fp);

    // Convert to network order and send to client
    networkFileSize = htonl(fileSize);
    sent = write(clientSocket, &networkFileSize, sizeof(networkFileSize));
    if (sent < 0)
    {
        perror("Error sending file size: \n");
    }

    // Seek back to the beginning
    rewind(fp);

    // Send the client the file
    bytesRead = 0;
    while ((bytesRead = fread(buffer, sizeof(char), sizeof(buffer), fp)) > 0)
    {
        // Make sure we have a successful send before incrementing our position
        offset = 0;
        while ((sent = write(clientSocket, buffer + offset, bytesRead)) > 0)
        {
            // If we sent something, increment our positions
            if (sent > 0)
            {
                offset += sent;
                bytesRead -= sent;
            }
        }
    }

    // Close our file and notify user operation is completed
    printf("File successfully sent!\n\n");
    fclose(fp);
}


// Send a message to a client socket connection
// This function is used when seding HTTP response codes (other than 200)
// Accepts a message pointer and a socket point, and does not return
void sendMessage(const char* message, int clientSocket)
{
    int n;                      // Number of bytes sent
    uint32_t convertedMsgSize;  // Size of the message we're sending


    // Send client message size
    convertedMsgSize = htonl(strlen(message));
    n = write(clientSocket, &convertedMsgSize, sizeof(convertedMsgSize));
    if (n < 0)
    {
        perror("Error sending client message size\n");
    }

    // Send message
    n = (write(clientSocket, message, strlen(message)));
    if (n < 0)
    {
        perror("Error sending client response\n");
    }
}
