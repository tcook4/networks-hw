#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#include <math.h>

#define BUFLEN 512  //Max length of buffer
#define PORT 6700   //The port on which to listen for incoming data

#define MAXSIZE 1000

struct stack
{
    char data[MAXSIZE];
    int index;
};

void convert(char *buffer);

void evaluate(char *buffer);

void push(char data, struct stack s);

char pop(struct stack s);

char peek(struct stack s);

int isEmpty(struct stack s);

int isOperand(char c);

int precedence(char c);

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

        // Convert buffer to postfix
        convert(buf);

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

// Convert infix to postfix
void convert(char *buffer)
{
    char input;
    char result[BUFLEN];
    struct stack s;
    s.index = -1;
    int i;

    // Iterate through our expression
    for(i=0; i < strlen(buffer); i++)
    {
        input = buffer[i];

        // If the character is an operand, append it to the result
        if (isOperand(input))
        {
            strcat(result, input);
        }

        // Operators get pushed to the stack
        else
        {
            // If we find an open parenthesis, push it to the stack
            if (input == '(')
            {
                push(input, s);
            }

            // If we find a closing parenthesis, pop the stack until we find the partner
            else if (input == ')')
            {
                while (stack.top() != '(')
                {
                    result+=stack.remove();
                }

                // Pop the '(' off the stack after we find it
                stack.pop();
            }

            // Pop operators with higher precidence before pushing
            else if ((precedence(stack.top())) >= (precedence(input[i])))
            {
                while (((precedence(stack.top())) >= (precedence(input[i]))) && (stack.size() >= 0))
                {
                    result+=stack.remove();
                }
                stack.push(input[i]);
            }

            // Otherwise, push the operator
            else
            {
                stack.push(input[i]);
            }
        }
    }

    // Finish by popping all remaining operators
    while (stack.size() >= 0)
    {
        result+=stack.remove();
    }


}




void evaluate(char *buffer)
{
    int temp1, temp2, temp3, i;
    char character;

    struct stack s;
    s.index = -1;


    for (i = 0; i <= strlen(buffer); i++)
    {
        character = buffer[i];

        // Push operands to the stack
        if (((character - 48) >= 0) && ((character -48) <= 9))
        {
            push(character-48, s);
        }

        // If operator, pop operands and evaluate, and push result back on to stack
        else
        {
            switch(character)
            {
            case '+':
                temp1 = pop(s);
                temp2 = pop(s);
                temp3 = temp2 + temp1;
                push(temp3, s);
                break;

            case '-':
                temp1 = pop(s);
                temp2 = pop(s);
                temp3 = temp2 - temp1;
                push(temp3, s);
                break;

            case '*':
                temp1 = pop(s);
                temp2 = pop(s);
                temp3 = temp2 * temp1;
                push(temp3, s);
                break;

            case '/':
                temp1 = pop(s);
                temp2 = pop(s);
                temp3 = temp2 / temp1;
                push(temp3, s);
                break;

            case '^':
                temp1 = pop(s);
                temp2 = pop(s);
                temp3 = temp2 ^ temp1;
                push(temp3, s);
                break;
            }
        }
    }
}


void push(char data, struct stack s)
{
    s.index++;
    s.data[s.index] = data;
}

char pop(struct stack s)
{
    s.index--;
    return s.data[s.index+1];
}

int isOperand(char c)
{
    if (c >= '0' && c <= '9')
    {
        return 1;
    }
    return 0;
}

int precedence(char c)
{
    if(c == '+' || c == '-')
    {
        return 0;
    }
    if(c == '*' || c == '/')
    {
        return 1;
    }
    if(c == '^')
    {
        return 2;
    }
    return -1;
}

char peek(struct stack s)
{
    return s.data[s.index];
}

int isEmpty(struct stack s)
{
    if (s.index == -1)
    {
        return 1;
    }
    return 0;
}
