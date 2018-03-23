/* Author: Tyler Cook
 * Date: 23 March, 2018
 * UNT CSCE 3530
 * Description: This is the server for a math expression parser. The server supports ten operations: addition, subtraction,
 * multiplication, division, square root, power (xy), exponential (ex), sine of a radian angle, cosine of a radian angle,
 * and log.
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <math.h>

#define BUFLEN 512  //Max length of buffer
#define MAXSIZE 1000 // Max size of our stack

// Char stack for converting expressions to postfix
typedef struct Stack
{
    char data[MAXSIZE];
    int index;
} Stack;

// Int stack for evaluating postfix expression
typedef struct intStack
{
    double data[MAXSIZE];
    int index;
} intStack;

// Function declarations, see below for descriptions
void convert(char *buffer);
void evaluate(char *buffer);
void push(char data, Stack *s);
char pop(Stack *s);
char peek(Stack *s);
int isEmpty(Stack *s);
int isOperand(char c);
int precedence(char c);
void intPush(double i, intStack *s);
double intPop(intStack *s);
void die(char *s);

int ERROR = 0;  // Error detected in expression


// Main method
int main(int argc, char **argv)
{
    struct sockaddr_in si_me, si_other;                 // Socket structure
    int s, slen = sizeof(si_other), recv_len;           // Socket variables
    char buf[BUFLEN];                                   // Recieve buffer
    int portNumber;                                     // Port number to listen on

    char *quit = "quit";
    char *errMsg = "Invalid expression";

    // Verify we have our port number
    if (argc != 2)
    {
        //printf("Error: Program usage: %s port_number\n", argv[0]);
        //exit(1);
        portNumber = 6700;
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
        printf("Expression: %s\n" , buf);

        // Check for quit
        if(strstr(buf, quit) != NULL)
        {
            printf("Quit message recieved, exiting...\n");
            exit(0);
        }

        // Convert buffer to postfix
        convert(buf);

        if (ERROR)
        {
            if (sendto(s, errMsg, strlen(errMsg), 0, (struct sockaddr*) &si_other, slen) == -1)
            {
                die("sendto()");
            }
            ERROR = 0;
            continue;
        }

        // Evaluate postix expression now in our buffer
        evaluate(buf);

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

// Convert infix expression to postfix
void convert(char *buffer)
{
    char input;             // Evaluate expression char by char
    char result[BUFLEN * 2];    // Result of our conversion
    int pos = 0;            // Keep track of our position in the result

    bzero(result, BUFLEN * 2);

    struct Stack stack;     // Stack for conversion and initialization
    stack.index = -1;
    stack.data[0] == ' ';

    int i;                  // Iterator
    int openFound = 0;      // Nested parenthesis detection

    // Iterate through our expression
    for(i=0; i < strlen(buffer); i++)
    {
        // Grab a character
        input = buffer[i];

        // Discard spaces
        if (input == ' ')
        {
            continue;
        }

        // Operands get appended to the result
        if (isOperand(input))
        {
            result[pos] = input;
            pos++;
        }

        // Operators get pushed to the stack
        else
        {
            // If we find an open parenthesis, push it to the stack
            if (input == '(')
            {
                // If we're already in an open bracket state, we've got nested expressions
                if (openFound)
                {
                    ERROR = 1;
                    return;
                }
                push(input, &stack);
                openFound = 1;
            }

            // If we find a closing parenthesis, pop the stack until we find the partner
            else if (input == ')')
            {
                openFound = 0;
                while (peek(&stack) != '(')
                {
                    result[pos] = pop(&stack);
                    pos++;
                }

                // Pop the '(' off the stack after we find it
                pop(&stack);
            }

            // Pop operators with higher precidence before pushing
            else if (((precedence(peek(&stack))) >= (precedence(input))) && (isEmpty(&stack)) != 1)
            {
                while (((precedence(peek(&stack))) >= (precedence(input))) && (stack.index >= 0))
                {
                    result[pos] = pop(&stack);
                    pos++;
                }
                push(input, &stack);
            }

            // Otherwise, push the operator
            else
            {
                // Detect exponential
                if (input == 'e')
                {
                    result[pos] = buffer[i+2];
                    pos++;
                    result[pos] = 'e';
                    pos++;
                    i+=3;
                }

                // Detect log
                else if (input == 'l')
                {
                    if (buffer[i+1] == 'o' && buffer[i+2] == 'g')
                    {
                        result[pos] = buffer[i+4];
                        pos++;
                        result[pos] = 'l';
                        pos++;
                        i+=5;
                    }
                    else
                    {
                        ERROR = 1;
                        return;
                    }
                }

                // sin
                else if (input == 's' && buffer[i+1] == 'i' && buffer[i+2] == 'n')
                {
                    result[pos] = buffer[i+4];
                    pos++;
                    result[pos] = 's';
                    pos++;
                    i+=5;
                }

                // cosine
                else if (input == 'c')
                {
                    if (buffer[i+1] == 'o' && buffer[i+2] == 's')
                    {
                        result[pos] = buffer[i+4];
                        pos++;
                        result[pos] = 'c';
                        pos++;
                        i+=5;
                    }
                    else
                    {
                        ERROR = 1;
                        return;
                    }
                }

                // squareroot
                else if (input == 's')
                {
                    if (buffer[i+1] == 'q' && buffer[i+2] == 'r' && buffer[i+3] == 't')
                    {
                        result[pos] = buffer[i+5];
                        pos++;
                        result[pos] = 'q';
                        pos++;
                        i+=6;
                    }
                    else
                    {
                        ERROR = 1;
                        return;
                    }
                }
                else
                {
                    push(input, &stack);
                }
            }
        }
    }

    // Finish by popping all remaining operators
    while (stack.index >= 0)
    {
        result[pos] = pop(&stack);
        pos++;
    }

    // Now set buffer to our new postfix expression
    bzero(buffer, BUFLEN);
    for (i = 0; i <= strlen(result); i++)
    {
        buffer[i] = result[i];
    }

    // Zero out the rest of the buffer memory
    for (i = strlen(result)+1; i < BUFLEN; i++)
    {
        buffer[i] = '\0';
    }
}



// Evaluate a postfix expression
void evaluate(char *buffer)
{
    double temp1, temp2, temp3, result;     // Number Storage
    int i;                                  // Iterator
    char character;                         // Parse expression char by char

    struct intStack stack;                  // Stack of numerical values
    stack.index = -1;

    // Iterate through our expression
    for (i = 0; i <= strlen(buffer); i++)
    {
        character = buffer[i];

        // Push operands to the stack
        if (((character - 48) >= 0) && ((character -48) <= 9))
        {
            intPush(character-48, &stack);
        }

        // If operator, pop operands and evaluate, and push result back on to stack
        else
        {
            // Switch with the operational character
            switch(character)
            {
            // Addition
            case '+':
                temp1 = intPop(&stack);
                temp2 = intPop(&stack);
                temp3 = temp2 + temp1;
                intPush(temp3, &stack);
                break;

                // Subtraction
            case '-':
                temp1 = intPop(&stack);
                temp2 = intPop(&stack);
                temp3 = temp2 - temp1;
                intPush(temp3, &stack);
                break;

                // Multiplication
            case '*':
                temp1 = intPop(&stack);
                temp2 = intPop(&stack);
                temp3 = temp2 * temp1;
                intPush(temp3, &stack);
                break;

                // Division
            case '/':
                temp1 = intPop(&stack);
                temp2 = intPop(&stack);
                temp3 = temp2 / temp1;
                intPush(temp3, &stack);
                break;

                // Power
            case '^':
                temp1 = intPop(&stack);
                temp2 = intPop(&stack);
                temp3 = pow(temp2, temp1);
                intPush(temp3, &stack);
                break;

                // Exponential
            case 'e':
                temp1 = intPop(&stack);
                temp3 = exp(temp1);
                intPush(temp3, &stack);
                break;

                // Sine
            case 's':
                temp1 = intPop(&stack);
                temp3 = sin(temp1);
                printf("got %f for sin value\n", temp3);
                intPush(temp3, &stack);
                break;

                // Cosine
            case 'c':
                temp1 = intPop(&stack);
                temp3 = cos(temp1);
                intPush(temp3, &stack);
                break;

                // Log
            case 'l':
                temp1 = intPop(&stack);
                temp3 = log(temp1);
                intPush(temp3, &stack);
                break;

                // Square root
            case 'q':
                temp1 = intPop(&stack);
                temp3 = sqrt(temp1);
                intPush(temp3, &stack);
                break;
            }
        }
    }

    // Grab our result
    result = intPop(&stack);
    printf("Result of evaluation is %.4f\n", result);

    // Record this result in our buffer
    snprintf(buffer, BUFLEN, "%.4f", result);
}

// Push a character onto the stack
void push(char data, Stack *s)
{
    s->index++;
    s->data[s->index] = data;
}

// Pop a character off the stack
char pop(Stack *s)
{
    s->index--;
    return s->data[s->index+1];
}

// Return 1 if c is an operand
int isOperand(char c)
{
    if (c >= '0' && c <= '9')
    {
        return 1;
    }
    return 0;
}

// Return the operational precedence of an operator
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
    if(c == 'c' || c == 's' || c == 'l' || c == 'e')
    {
        return 3;
    }
    return -1;
}

// Peek at the top valueof the stack
char peek(Stack *s)
{
    if (s->index != -1)
    {
        return s->data[s->index];
    }
    return -1;
}

// Return 1 if the stack is empty
int isEmpty(Stack *s)
{
    if (s->index == -1)
    {
        return 1;
    }
    return 0;
}

// Push an integer onto the intstack
void intPush(double i, intStack *s)
{
    s->index++;
    s->data[s->index] = i;
}

// Pop an integer off an intstack
double intPop(intStack *s)
{
    s->index--;
    return s->data[s->index+1];
}

// Print error and exit
void die(char *s)
{
    perror(s);
    exit(1);
}

