/* Author: Tyler Cook
 * Date: 23 March, 2018
 * UNT CSCE 3530
 * Description
 *
 * Operaitons: The math server should be able to perform ten operations: addition, subtraction,
multiplication, division, square root, power (xy), exponential (ex), sine of a radian
angle, cosine of a radian angle, and log
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#include <math.h>

#define BUFLEN 512  //Max length of buffer
#define PORT 6700   //The port on which to listen for incoming data

#define MAXSIZE 1000

void evaluate(char *buf);


void die(char *s)
{
    perror(s);
    exit(1);
}

int main(void)
{
    struct sockaddr_in si_me, si_other;

    int s, i, slen = sizeof(si_other), recv_len;
    char buf[BUFLEN];

    //create a UDP socket
    if ((s=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1)
    {
        die("socket");
    }

    // zero out the structure
    memset((char *) &si_me, 0, sizeof(si_me));

    si_me.sin_family = AF_INET;
    si_me.sin_port = htons(PORT);
    si_me.sin_addr.s_addr = htonl(INADDR_ANY);

    //bind socket to port
    if( bind(s , (struct sockaddr*)&si_me, sizeof(si_me) ) == -1)
    {
        die("bind");
    }

    //keep listening for data
    while(1)
    {
        printf("Waiting for data...");
        fflush(stdout);

        //try to receive some data, this is a blocking call
        if ((recv_len = recvfrom(s, buf, BUFLEN, 0, (struct sockaddr *) &si_other, &slen)) == -1)
        {
            die("recvfrom()");
        }

        //print details of the client/peer and the data received
        printf("Received packet from %s:%d\n", inet_ntoa(si_other.sin_addr), ntohs(si_other.sin_port));
        printf("Data: %s\n" , buf);

        // Pass buffer to math function
        evaluate(buf);


        //now reply the client with the same data
        if (sendto(s, buf, recv_len, 0, (struct sockaddr*) &si_other, slen) == -1)
        {
            die("sendto()");
        }
    }

    close(s);
    return 0;
}


void evaluate(char *buf)
{

    int i;
    char input;
    char result[BUFLEN];
    char operation;

    char numStore1[50];
    char numStore2[50];

    int numIndex1 = 0;
    int numIndex2 = 0;


    for (i = 0; i < strlen(buf); i++)
    {
        // Skip spaces
        if (buf[i] == ' ')
        {
            continue;
        }

        // Store numbers for use later
        if (buf[i] >= '0' && buf[i] <= '9')
        {
            numStore1[numIndex1] = buf[i]-48;
            numIndex1++;
            printf("got number\n");
            continue;
        }

        else if (buf[i] = '(')
        {






        }
    }
}
