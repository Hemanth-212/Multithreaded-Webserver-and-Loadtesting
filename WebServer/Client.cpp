#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<string.h>
#include<netdb.h>
#include<pthread.h>

void printErrorMsg(const char *msg);

int main(int argc, char *argv[]){
    
    /* If no of arguments in commands are not 3 then print error message */
    if(argc != 3)
    {
        fprintf(stderr,"Correct usage is %s hostname port number\n",argv[0]);
        exit(0);
    }
    int cltSockFd, portNo, s;
    struct hostent *serverent; // To get the host details like domain type (AF_INET etc),address length and address
    struct sockaddr_in server_addr; // Stores information about socket like domian type, address, portno etc.

    /*
        Steps followed are 
        1. Create client socket
        2. Fill details of server
        3. Connect to server
        4. Send and receive messages to server
    */
    
    // 1. Create client socket

    cltSockFd = socket(AF_INET,SOCK_STREAM,0);

    if(cltSockFd < 0)
    {
        printErrorMsg("Error creating client's socket\n");
    }


    // 2. Fill details of server
    char *hostname = argv[1];
    serverent = gethostbyname(hostname);

    if(!serverent)
    {
        printErrorMsg("Can not find any such host\n");
    }

    portNo = atoi(argv[2]);
    bzero((char *)&server_addr,sizeof(server_addr)); // For writing \0 in n bytes 
    bcopy((char *)serverent->h_addr, (char *)&server_addr.sin_addr.s_addr,serverent->h_length);// Copy the host address
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(portNo);

    // 3. Connect to server

    if(connect(cltSockFd,(struct sockaddr *)&server_addr,sizeof(server_addr)) < 0)
    {
        printErrorMsg("ERROR connecting to server\n");
    }

    //4. Send and Recieve message to and from server

    char msgBuffer[256];

    while(1)
    {
        printf("Enter a message: \n");
        bzero(msgBuffer,256);
        fgets(msgBuffer,255,stdin);

        /* Send the message to buffer */
        int msgLen = strlen(msgBuffer);
        s= write(cltSockFd,msgBuffer,msgLen);
        if(s < 0)
        {
            printErrorMsg("ERROR in writing to server\n");
        }

        bzero(msgBuffer,256); // Clear buffer for further usage

        /* Read message from server*/

        s = read(cltSockFd,msgBuffer,256);
        if(s < 0)
        {
            printErrorMsg("ERROR reading message from server\n");
        }
        
        printf("Server message is : %s\n",msgBuffer);
        bzero(msgBuffer,256);
    }

    close(cltSockFd);
    return 0;

}

void printErrorMsg(const char *msg){
    perror(msg);
    exit(0);
}