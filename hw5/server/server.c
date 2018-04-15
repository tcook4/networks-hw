/* Author: Tyler Cook
 * Date: 25 April, 2018
 * UNT CSCE 3530
 * Description:
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <math.h>

#define BUFLEN 512  //Max length of buffer
#define MAXSIZE 1000 // Max size of our stack

// Function declarations, see below for descriptions

void die(char *s);

int ERROR = 0;  // Error detected in expression


// Main method
int main(int argc, char **argv)
{
    struct sockaddr_in si_me, si_other;                 // Socket structure
    int s, slen = sizeof(si_other), recv_len;           // Socket variables
    char buf[BUFLEN];                                   // Recieve buffer
    int portNumber;                                     // Port number to listen on

    // Default messages
    char *quit = "quit";
    char *errMsg = "Invalid expression";

    // Verify we have our port number
    if (argc != 2)
    {
        printf("Error: Program usage: %s port_number\n", argv[0]);
        exit(1);
    }
    else
    {
        portNumber = atoi(argv[1]);
    }

    //create a UDP socket
    if ((s=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1)
    {
        die("Socket error");
    }

    // Zero out the structure
    memset((char *) &si_me, 0, sizeof(si_me));
    si_me.sin_family = AF_INET;
    si_me.sin_port = htons(portNumber);
    si_me.sin_addr.s_addr = htonl(INADDR_ANY);

    // Bind socket to given port mumber
    if( bind(s , (struct sockaddr*)&si_me, sizeof(si_me) ) == -1)
    {
        die("Bind error");
    }

    // Run forever listening for data
    while(1)
    {
        bzero(buf, BUFLEN);
        printf("Waiting for data...");
        fflush(stdout);

        // Recieve data from client
        if ((recv_len = recvfrom(s, buf, BUFLEN, 0, (struct sockaddr *) &si_other, &slen)) == -1)
        {
            die("recvfrom()");
        }

        // Print expression as recieved
        printf("Expression recieved: %s\n" , buf);

        // Check for quit
        if(strstr(buf, quit) != NULL)
        {
            printf("Quit message recieved, exiting...\n");
            exit(0);
        }

        // Reply to the client with our result
        if (sendto(s, buf, strlen(buf), 0, (struct sockaddr*) &si_other, slen) == -1)
        {
            die("sendto()");
        }
    }

    // Close the socket
    close(s);
    return 0;
}

// Print error and exit
void die(char *s)
{
    perror(s);
    exit(1);
}
