/* Author: Tyler Cook
 * Date: 23 March, 2018
 * UNT CSCE 3530
 * Description: This is the client to a math expression server. The server supports multiple operations, including
 * addition, subtraction, multiplication, division, square root, power (xy), exponential (ex), sine of a radian
 * angle, cosine of a radian angle, and log.
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define SERVER "129.120.151.94"
#define BUFLEN 512  //Max length of buffer

void die(char *s)
{
    perror(s);
    exit(1);
}

int main(int argc, char **argv)
{
    struct sockaddr_in si_other;
    int s, i, slen=sizeof(si_other);
    char buf[BUFLEN];
    char message[BUFLEN];
    char *quit = "quit";
    int portNumber;

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

    // Set our socket
    if ( (s=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1)
    {
        die("socket");
    }

    // Set structure details
    memset((char *) &si_other, 0, sizeof(si_other));
    si_other.sin_family = AF_INET;
    si_other.sin_port = htons(portNumber);

    if (inet_aton(SERVER , &si_other.sin_addr) == 0)
    {
        fprintf(stderr, "inet_aton() failed\n");
        exit(1);
    }

    // Loop forever
    while(1)
    {
        // Get message from user
        printf("Enter expression: ");
        gets(message);

        // Send message to the client
        if (sendto(s, message, strlen(message) , 0 , (struct sockaddr *) &si_other, slen)==-1)
        {
            die("sendto()");
        }

        // Check if we're quitting
        if (strstr(message, quit) != NULL)
        {
            printf("Exiting...\n");
            exit(0);
        }

        // Try and recieve our result from the server
        printf("Result: ");
        memset(buf,'\0', BUFLEN);
        if (recvfrom(s, buf, BUFLEN, 0, (struct sockaddr *) &si_other, &slen) == -1)
        {
            die("recvfrom()");
        }

        // Print the response
        puts(buf);
    }

    // Close the socket and exit
    close(s);
    return 0;
}
