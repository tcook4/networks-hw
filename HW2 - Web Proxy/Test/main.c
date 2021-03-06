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

int main(int argc, char **argv)
{
    char buffer[1024];                  // Communication buffer between client and server
    int listen_fd, conn_fd, n;          // File descriptors and error checking
    struct sockaddr_in servaddr;        // Server address structure
    char output[1024];                  // Output message storage
    int bufferLength;                   // Length of message to be sent
    int dataLength;                     // Length of message to be received
    int portNumber;                     // Port number to use if supplied

    FILE *fp;                           // File pointer for
    int webSockFD;                      // File descriptor for our web server

    char address[100];
    char *formattedAddress;
    char *match = "http://";
    char *match2 = "https://";


    // Get allow list from file

    char* message = "GET /\r\n HTTP /1.1.";

    for(;;) // run forever
    {
        do
        {

            printf("Enter website: ");

            bzero(address, 100);
            scanf("%s", &address);

            // Remove http(s) from address for gethostbyname to work
            // This is kind of hacky
            formattedAddress = strtok(address, match);
            if (formattedAddress == NULL) // If we didn't find http, look for https
            {
                formattedAddress == strtok(address, match2);
            }
            if (formattedAddress == NULL) // If we didn't find either
            {
                formattedAddress == address;
            }

            // Go get the webpage
            printf("Trying to connect to website\n");
            webSockFD = connectWebServer(formattedAddress);

            if (webSockFD == -1)
            {
                printf("Connection failed, please try again.\n");
            }
        }
        while (webSockFD == -1);


        write(webSockFD, message, sizeof(message));

        int bytesRead = 0;
        fp = fopen(formattedAddress, "w");

        do
        {

            bzero(buffer, sizeof(buffer));

            bytesRead = read(webSockFD, buffer, sizeof(buffer));

            fprintf(fp, "%s", buffer);
        }

        while (bytesRead > 0);


        printf("Done reading\n");

        // Get by hostname


        fclose(fp);




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

    printf("Connecting to %s\n", website);

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
