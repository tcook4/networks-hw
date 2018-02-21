Author: Tyler Cook
Date: February 5th, 2018
Course: UNT CSCE 3530
Programming Assignment #1

README

This program consists of two files - server.c and client.c. Server.c runs on the UNT CSE 01 machine, and client can run on
any UNT CSCE machine. To compile the programs, use gcc and the filename. IE: gcc server.c. The attached makefile can be used, or
see the below example for the makefile contents.

all: server client

server: server.c
    gcc -o server server.c
client: client.c
    gcc -o client client.c


To run the program, type the server executible name followed by the port number, example:
./server 22000
Then start the client using the same port number, example:
./client 22000

On the client machine, type a string followed by enter to get the word count and lower-case version from the server.
You can also type 'quit' to exit the client and server.
