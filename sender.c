#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netinet/tcp.h>
#include <stdbool.h>

#define SIZE 10000
#define SERVER_PORT 5060              // The port that the server listens
#define SERVER_IP_ADDRESS "127.0.0.1" // The IP address
#define id1 9647
#define id2 3367

void SendFirst(FILE *fr, int counterBytes, int counterSegments, int sock, int sumChar);
void SendSecond(FILE *fr, int counterBytes, int counterSegments, int sock, int sumChar);

int main() {
    // Read the file
    FILE *fr;
    fr = fopen("text.txt", "r");
    if (fr == NULL) {
        perror("Error in reading file.\n");
        exit(1);
    }

    //Count the chars in the file
    int sumChar = 0;
    fseek(fr, 0, SEEK_END);
    sumChar = ftell(fr);
    rewind(fr);

    //Creat connection
    int listeningSocket, clientSocket;
    if ((listeningSocket = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        printf("Could not create listening socket : %d\n", errno);
    }
    int enableReuse = 1;
    if (setsockopt(listeningSocket, SOL_SOCKET, SO_REUSEADDR, &enableReuse, sizeof(int)) < 0) {
        printf("setsockopt() failed with error code : %d\n", errno);
    }

    struct sockaddr_in serverAddress;
    memset(&serverAddress, 0, sizeof(serverAddress));
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(SERVER_PORT);

    //Bind the socket
    if (bind(listeningSocket, (struct sockaddr *) &serverAddress, sizeof(serverAddress)) < 0) {
        printf("Couldn't bind to the port\n");
        return -1;
    }
    printf("server is binding\n\n");

    if (listen(listeningSocket, 500) == -1) {
        printf("listen() failed with error code : %d\n", errno);
        close(listeningSocket);
        return -1;
    }
    struct sockaddr_in clientAddress;
    socklen_t clientAddressLen = sizeof(clientAddress);

    //Accept and incoming connection
    memset(&clientAddress, 0, sizeof(clientAddress));
    clientAddressLen = sizeof(clientAddress);
    clientSocket = accept(listeningSocket, (struct sockaddr *)&clientAddress, &clientAddressLen);

    if (clientSocket== -1)
    {
        printf("listen failed with error code : %d", errno);
        close(listeningSocket);
        return -1;
    }
    int counterBytes = 0;
    int counterSegments = 0;
    bool flag = true;

    while (1)
    {
        int tempSend = send(clientSocket, &sumChar, sizeof(sumChar),0);
        if(tempSend > 0)
        {
            printf("The size of the file sent succefully\n");
        }
        else{
            printf("Error in send\n");
        }

        //Change coongestion control to cubic
        printf("\nChanged congestion control to cubic\n");
        char changeBuf1[6];
        strcpy(changeBuf1,"cubic");
        socklen_t lenBuf1 = strlen(changeBuf1);
        if(setsockopt(listeningSocket,IPPROTO_TCP, TCP_CONGESTION, changeBuf1, lenBuf1) != 0)
        {
            perror("There is a problem in setsockopt");
            return -1;
        }
        lenBuf1 = sizeof(changeBuf1);
        if(getsockopt(listeningSocket,IPPROTO_TCP, TCP_CONGESTION, changeBuf1, &lenBuf1) != 0)
        {
            perror("There is a problem in getsockopt");
            return -1;
        }
        printf("Changed congestion control to 'cubic' have succed\n\n");

        // send first part of file
        rewind(fr);
        SendFirst(fr, counterBytes, counterSegments, clientSocket, sumChar);

        printf("\nFirst half of file data sent successfully.\n");
        //Done send the first half of the file

        //Getting the autotentection
        int autSend = id1 ^ id2;
        int autRecv;
        int sizeAuth = recv(clientSocket, &autRecv, sizeof(autRecv), 0);
        printf("The sender calculate %d ,The sender received: %d\n", autSend, autRecv);
        if (autSend == autRecv)
        {
            printf("Authotentection have prooved\n\n");
        }
        else
        {
            printf("Autotentection have failed\n\n");
            exit(1);
        }

        //Change the cc Algorithm to reno
        printf("Changed congestion control to reno\n");
        char changeBuf2[5];
        strcpy(changeBuf2,"reno");
        socklen_t lenBuf2 = strlen(changeBuf2);
        if(setsockopt(clientSocket,IPPROTO_TCP, TCP_CONGESTION, changeBuf2, lenBuf2) != 0)
        {
            perror("There is a problem in setsockopt\n");
            return -1;
        }
        lenBuf2 = sizeof(changeBuf2);
        if(getsockopt(clientSocket,IPPROTO_TCP, TCP_CONGESTION, changeBuf2, &lenBuf2) != 0)
        {
            perror("There is a problem in getsockopt\n");
            return -1;
        }
        printf("Changed congestion control to 'reno' have succed\n");

        //Sending the second part of the file.
        SendSecond(fr, counterBytes, counterSegments, clientSocket, sumChar);
        printf("Second half of file data sent successfully.\n\n");

        //asking the user if he wants to send again
        int c;
        printf("Do you wont to sending again? [0/1]\n\n");
        scanf("%d",&c);
        if (c == 1)
        {
            break;
        }

    }

    int sendExit = -300;
    int tempSend = send(clientSocket, &sendExit, sizeof(sendExit),0);
    if(tempSend > 0)
    {
        printf("The exit sent succefully\n");
    }
    else{
        printf("Error in send\n");
    }
    close(clientSocket);
    fclose(fr);
    close(listeningSocket);

    return 0;
}

//function

void SendFirst(FILE *fr, int counterBytes, int counterSegments, int sock, int sumChar) {
    rewind(fr);
    char buffer[sumChar / 2];
    char sendExit[5];
    fread(buffer, 1, sumChar / 2, fr);
    if (send(sock, buffer, sizeof(buffer), 0) == -1) {
        perror("Er\n");
        exit(1);
    }
}

void SendSecond(FILE *fr, int counterBytes, int counterSegments, int sock, int sumChar) {
    char buffer[sumChar / 2];
    char sendExit[5];
    fread(buffer, 1, sumChar / 2, fr);
    if (send(sock, buffer, sizeof(buffer), 0) == -1) {
        perror("Er\n");
        exit(1);
    }
    bzero(buffer, sumChar / 2);
}