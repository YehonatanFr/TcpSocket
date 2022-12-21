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
#include <time.h>
#include <sys/time.h>
#include <stdbool.h>

#define SERVER_IP_ADDRESS "127.0.0.1" // The IP address
#define SERVER_PORT 5060 // The port that the server listens
#define SIZE 10000
#define id1 9647
#define id2 3367

int main() {
    //Open listening and conncetion
    char buffer[SIZE];

    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        printf("Unable to create socket\n");
        return -1;
    }

    struct sockaddr_in serverAddress;
    memset(&serverAddress, 0, sizeof(serverAddress));

    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = INADDR_ANY;
    serverAddress.sin_port = htons(SERVER_PORT);
    int rval = inet_pton(AF_INET, (const char *) SERVER_IP_ADDRESS, &serverAddress.sin_addr);
    if (rval <= 0) {
        printf("inet_pton() failed");
        return -1;
    }
    if (connect(sock, (struct sockaddr *) &serverAddress, sizeof(serverAddress)) == -1) {
        printf("connect() failed with error code : %d", errno);
    }
    printf("connected to server\n");

    struct sockaddr_in clientAddress; //
    socklen_t clientAddressLen = sizeof(clientAddress);

    double Times[50];
    int i =0; //Pointer for the array
    double TimeFirstHalf, TimeSecondHalf;
    int counterFileReceive = 0;
    double seconds,microseconds,finResult;

    int sumChar = 0;

    int RecivedBytesCounter = 0;
    int counterSegments = 0;

    while (1)
    {
        int return_status = recv(sock, &sumChar, sizeof(sumChar),0);
        if (return_status > 0) {
            printf("Get size of the file succed \n\n");
        }
        if(sumChar == -300)
        {
            printf("Asking for Exit have received\n");
            break;
        }

        //Change coongestion control to cubic
        printf("Changed congestion control to cubic\n");
        char changeBuf1[6];
        strcpy(changeBuf1, "cubic");
        socklen_t lenBuf1 = strlen(changeBuf1);
        if (setsockopt(sock, IPPROTO_TCP, TCP_CONGESTION, changeBuf1, lenBuf1) != 0)
        {
            perror("There is a problem in setsockopt");
            return -1;
        }
        lenBuf1 = sizeof(changeBuf1);
        if (getsockopt(sock, IPPROTO_TCP, TCP_CONGESTION, changeBuf1, &lenBuf1) != 0)
        {
            perror("There is a problem in getsockopt");
            return -1;
        }
        printf("Changed congestion control to cubic have succed\n\n");

        bool StartTime = false;
        struct timeval begin,end; //Structs for measure times
        char getExit[5] = {0};

        //Receive the first half
        while(RecivedBytesCounter < (sumChar/2)) {
            bzero(buffer, SIZE);
            int recv_check = recv(sock, buffer, SIZE, 0);
            if (recv_check < 0) {
                printf("Recv segment has failed\n");
                break;
            } else if (recv_check == 0) {
                printf("Cant received socket closed\n");
            }
            if(StartTime == false)
            {
                StartTime = true;
                gettimeofday(&begin, 0);
            }

            RecivedBytesCounter= RecivedBytesCounter+strlen(buffer);
            counterSegments++;
        }
        gettimeofday(&end, 0); //Save the current time

        printf("\nFirst half of file received successfully.\n");

        //Some prints info for checks
        printf("The num of segments get: %d\n",counterSegments);
        printf("Total bytes recived: %d\n", RecivedBytesCounter);
        printf("\n\n");

        //Take care about the time
        seconds = (double)(end.tv_sec - begin.tv_sec);
        microseconds = (double)(end.tv_usec - begin.tv_usec);
        finResult = seconds + microseconds*1e-6;
        Times[i] = finResult;
        i++;
        TimeFirstHalf += finResult;

        //Send authotentection
        printf("Try to send autotentection\n");
        int auth = id1^id2;
        int resultSend = send(sock,&auth,sizeof(auth), 0);
        printf("Authotentection have checked\n\n");

        //Change cc Algorithm to reno
        printf("Changed congestion control to reno\n");
        char changeBuf2[5];
        strcpy(changeBuf2, "reno");
        socklen_t lenBuf2 = strlen(changeBuf2);
        if (setsockopt(sock, IPPROTO_TCP, TCP_CONGESTION, changeBuf2, lenBuf2) != 0)
        {
            perror("There is a problem in setsockopt\n");
            return -1;
        }
        lenBuf2 = sizeof(changeBuf2);
        if (getsockopt(sock, IPPROTO_TCP, TCP_CONGESTION, changeBuf2, &lenBuf2) != 0)
        {
            perror("There is a problem in getsockopt\n");
            return -1;
        }
        printf("Changed congestion control to reno have succed");

        //Receive the second half
        counterSegments = 0;
        int RecivedBytesCounter2 = 0;
        StartTime = false;
        while(RecivedBytesCounter2 < sumChar/2 )
        {
            bzero(buffer,SIZE);
            int recv_check = recv(sock, buffer, SIZE, 0);
            if (recv_check < 0)
            {
                printf("recv segment has failed\n");
                break;
            }
            else if(recv_check == 0)
            {
                printf("Cant received socket closed\n");
            }
            //Save the current time
            if(StartTime == false)
            {
                StartTime = true;
                gettimeofday(&begin, 0);
            }

            RecivedBytesCounter2 = RecivedBytesCounter2 +strlen(buffer);
            counterSegments++;
        }
        //New tab
        printf("\n\n");

        gettimeofday(&end, 0); //Save the current time
        
        printf("\nSecond half of file received successfully.\n");
        
        //Take care about the time
        seconds = (double)(end.tv_sec - begin.tv_sec);
        microseconds = (double)(end.tv_usec - begin.tv_usec);
        finResult = seconds + microseconds*1e-6;
        Times[i] = finResult;
        i++;
        TimeSecondHalf += finResult;
        counterFileReceive++;

        printf("The num of segments : %d\n",counterSegments);
        printf("Total bytes recived: %d\n", RecivedBytesCounter);
        printf("\n\n");

        RecivedBytesCounter = 0;
        counterSegments = 0;
        printf("Get the file for %d time\n\n", counterFileReceive);
    }
    //Time printing
    printf("\nPrint the times of both halfs \n");

    int h = 1;
    for(int j=0; j<counterFileReceive*2; j+=2)
    {
        printf("%d - Times of first half (With cubic cc): %f\t Times for second half (With reno cc):%f\n\n",h, Times[j], Times[j+1]);
        h++;
    }

    //Print average of times
    printf("Print Average of Times\n");
    printf("Average times for the first part: %lf Sec\n", TimeFirstHalf/counterFileReceive);
    printf("Average times for the second part: %lf Sec\n", TimeSecondHalf/counterFileReceive);
    printf("Total times:%lf\n\n", (TimeFirstHalf+TimeSecondHalf)/2);
    
    return 0;
}
