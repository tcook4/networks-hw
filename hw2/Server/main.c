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

int main(int argc, char **argv)
{
    char buffer[1024];                  // Communication buffer between client and server
    int listenFd, clientFd, n;          // File descriptors and error checking
    struct sockaddr_in servaddr;        // Server address structure
    char output[1024];                  // Output message storage
    int bufferLength;                   // Length of message to be sent
    int dataLength;                     // Length of message to be received
    int portNumber;                     // Port number to use if supplied
    int connected = 0;                  // Connection status to client

    FILE *fp;                           // File pointer for
    int webSockFD;                      // File descriptor for our web server


    char* message = "GET /\r\n HTTP /1.1.";
    char response[100] = ""; // Storage for first line of http response
    char address[100];
    char formattedAddress[100];
    char* searchPtr;
    char *responseCode;


    /*
    // Connect to client
    // AF_INET - IPv4 IP , Type of socket, protocol
    listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htons(INADDR_ANY);
    servaddr.sin_port = htons(portNumber);

    // Binds the above details to the socket
    bind(listen_fd, (struct sockaddr *) &servaddr, sizeof(servaddr));

    // Start listening to incoming connections
    listen(listen_fd, 10);
    printf("Listening for connection...\n");

    // Accept an incoming connection
    conn_fd = accept(listen_fd, (struct sockaddr*)NULL, NULL);
    if (conn_fd < 0) // verify we accepted correctly
    {
        perror("ERROR on accept");
        exit(1);
    }

    printf("Successfully connected to client!\n");

    */
    connected = 1;


    // Get allow list from file

    while (connected)
    {
        do
        {
            // Read website from client
            printf("Enter website: ");

            bzero(address, 100);
            bzero(formattedAddress, 100);
            scanf("%s", &address);

            // Remove http(s) from address for gethostbyname to work
            if ((searchPtr = strstr(address, "http://")))
            {
                strcpy(formattedAddress, &address[7]);
            }
            else if ((searchPtr = strstr(address, "https://")))
            {
                strcpy(formattedAddress, &address[8]);
            }
            else // If we didn't find either
            {
                formattedAddress = address;
            }


            // TODO: check cache here for cached version



            // Connect to the webpage
            printf("Trying to connect to website\n");
            webSockFD = connectWebServer(formattedAddress);

            if (webSockFD == -1)
            {
                printf("Connection failed, please try again.\n");
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
        fp = fopen(searchPtr, "w+");
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
            sendFile(formattedAddress, clientFd);

            // Append to list.txt



        }
        else
        {
            // Send response
            printf("Webserver response: %s\n", response);

            // Close file and remove cached version
            fclose(fp);
            remove(formattedAddress);
        }




        // Check if response code is 200
        // If 200, cashe and foward to user
        // Else, foward response

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

}
