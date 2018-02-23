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

int connectWebServer(char* website);

void sendFile(char* filename, int clientSocket);

void sendMessage(const char *message, int clientSocket);

int main(int argc, char **argv)
{
    char buffer[1024];                  // Communication buffer between client and server
    int listenFd, clientSocket, n;          // File descriptors and error checking
    struct sockaddr_in servaddr;        // Server address structure
    char output[1024];                  // Output message storage
    int bufferLength;                   // Length of message to be sent
    int dataLength;                     // Length of message to be received
    int portNumber;                     // Port number to use if supplied
    int connected = 0;                  // Connection status to client
    int msgSize;
    uint32_t convertedMsgSize; // converted message size

    FILE *fp;                           // File pointer for
    int webSockFD;                      // File descriptor for our web server


    char* message = "GET /\r\n HTTP /1.1.";
    char response[100] = ""; // Storage for first line of http response
    char address[1024];
    char formattedAddress[100];
    char* searchPtr;
    char *responseCode;

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

    portNumber = 8855;


    // Connect to client
    // AF_INET - IPv4 IP , Type of socket, protocol
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
    clientSocket = accept(listenFd, (struct sockaddr*)NULL, NULL);
    if (clientSocket < 0) // verify we accepted correctly
    {
        perror("ERROR on accept");
        exit(1);
    }
    printf("Successfully connected to client!\n");
    connected = 1;


    // Get allow list from file

    while (connected)
    {
        do
        {
            // Initialize our buffers
            bzero(address, 1024);
            bzero(formattedAddress, 100);
            bzero(buffer, 1024);

            // Get message size and convert from network order
            n = read(clientSocket, (char*)&bufferLength, sizeof(bufferLength));
            if (n < 0)
            {
                perror("Error getting data size\n");
            }
            dataLength = ntohl(bufferLength);

            // Read the website address
            bzero(buffer, 1024);
            n = read(clientSocket, buffer, dataLength);
            if (n < 0)
            {
                perror("Error reading message from client\n");
            }
            strcpy(address, buffer);
            printf("address is %s\n", address);


            // Remove http(s) from address for gethostbyname to work
            if ((searchPtr = strstr(address, "http://")))
            {
                strncpy(formattedAddress, &address[7], 100);
            }
            else if ((searchPtr = strstr(address, "https://")))
            {
                strncpy(formattedAddress, &address[8], 100);
            }
            else // If we didn't find either
            {
                strncpy(formattedAddress, address, 100);
            }
            printf("formatted address is %s\n", formattedAddress);



            // TODO: check cache here for cached version


            // Connect to the webpage
            printf("Trying to connect to website\n");
            webSockFD = connectWebServer(formattedAddress);

            if (webSockFD == -1)
            {
                printf("Connection failed, please try again.\n");
                sendMessage("Error, please try again", clientSocket);
            }
        }
        while (webSockFD == -1);

        // Send GET request to web server
        n = write(webSockFD, message, sizeof(message));
        if (n < 0)
        {
            printf("Error writing to website...\n");
            continue;
        }

        // Open a file in preparation to recieve data
        int bytesRead = 0;
        fp = fopen(formattedAddress, "w+");
        if (fp == NULL)
        {
            perror("Error opening file\n");
        }

        // Read response from web server
        do
        {
            // Zero our buffer, read from the web page, and store buffer in our file
            bzero(buffer, sizeof(buffer));
            bytesRead = read(webSockFD, buffer, sizeof(buffer));
            fprintf(fp, "%s", buffer);
        }
        while (bytesRead > 0);
        printf("Webpage retrieved\n");

        // Rewrind to beginning of file and grab first line
        fseek(fp, 0, SEEK_SET);
        fgets(response, 100, fp);

        // Check response code
        if (strstr(response, "200 OK"))
        {
            printf("Response code 200 - sending webpage to client\n");
            fclose(fp);
            sendFile(formattedAddress, clientSocket);

            // Append to list.txt



        }
        else
        {
            // Response code not 200 - send response and delete cached version
            printf("Response code not 200... response: %s\n", response);

            sendMessage(response, clientSocket);

            // Close file and remove cached version
            fclose(fp);
            remove(formattedAddress);
        }


        // Get website info and put into list.txt


    }
    return 0;
}

int connectWebServer(char *website)
{
    int sockfd;
    struct sockaddr_in serv_addr;             // socket structure
    struct hostent *server;                   // socket host struct

    printf("Connecting to %s...\n", website);

    // create a socket point
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
    {
        perror("ERROR opening socket");
        return -1;
    }

    // assign and address our server
    server = gethostbyname(website);
    if (server == NULL)
    {
        fprintf(stderr,"ERROR, no such host\n");
        return -1;
    }

    // fill in our server address struct
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr, server->h_length);
    serv_addr.sin_port = htons(80);

    // connect to our server
    if (connect(sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0)
    {
        perror("ERROR connecting");
        return -1;
    }

    printf("Successfully connected to web server on port 80.\n");

    return sockfd;
}

void sendFile(char *filename, int clientSocket)
{
    int fileSize; // Length of the file we're finding
    FILE *fp;   // File pointer
    int n; // Write bits
    char buffer[1024]; // Buffer
    uint32_t networkFileSize;
    int bytesRead;
    int offset; // keep track of our file position


    printf("Sending file to client...\n");
    // Open file
    fp = fopen(filename, "r");
    if (fp == NULL)
    {
        perror("File open failed: \n");
    }

    // Seek to the end and find file length
    fseek(fp, 0, SEEK_END);
    fileSize = (int)ftell(fp);

    // Convert to network order and send to client
    networkFileSize = htonl(fileSize);
    n = write(clientSocket, &networkFileSize, sizeof(networkFileSize));
    if (n < 0)
    {
        perror("Error sending file size: \n");
    }

    // Seek back to the beginning
    rewind(fp);

    // Send the client the file
    printf("Preparing to send file...\n");
    bytesRead = 0;
     while ((bytesRead = fread(buffer, sizeof(char), sizeof(buffer), fp)) > 0)
     {
         // Make sure we have a successful send before incrementing our position
         offset = 0;
         while ((n = write(clientSocket, buffer + offset, bytesRead)) > 0)
         {
             // If we sent something, increment our positions
             if (n > 0)
             {
                 offset += n;
                 bytesRead -= n;
             }
         }
     }
     // close our file and notify user operation is completed
     printf("File successfully sent!\n\n");

    fclose(fp);
}


void sendMessage(const char* message, int clientSocket)
{
    int n;
    uint32_t convertedMsgSize;

    printf("sending : %d\n", strlen(message));

    // Send client response size
    convertedMsgSize = htonl(strlen(message));
    n = write(clientSocket, &convertedMsgSize, sizeof(convertedMsgSize));
    if (n < 0)
    {
        perror("Error sending client message size\n");
    }

    // Send response
    n = (write(clientSocket, message, strlen(message)));
    if (n < 0)
    {
        perror("Error sending client response\n");
    }

}
