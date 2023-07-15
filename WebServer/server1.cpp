#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<string.h>
#include<netdb.h>
#include<pthread.h>


void printErrorMsg(const char *msg);
void *client_handler(void *arg);

int main(int argc, char *argv[])
{
    if(argc != 2)
    {
        printErrorMsg("Enter port number\n");
    }

    struct sockaddr_in  server_addr, client_addr;
    int servSockFd, clientSockFd, portNo;
    socklen_t cltlen;

    /* Steps followed
        1. Create server listen socket
        2. Fill in server details
        3. Bind the server socket to port number
        4. Listen to new incoming requests
        5. Accept and create a new socket for client and assign a thread
        6. Read and write from and to client
    */

   // 1. Create a server listen socket

   servSockFd = socket(AF_INET, SOCK_STREAM,0);
   if(servSockFd < 0)
   {
        printErrorMsg("ERROR in creating a server socket\n");
   }

   // 2. Create a server listen socket
    portNo = atoi(argv[1]);
    bzero((char *)&server_addr,sizeof(server_addr));
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(portNo);
    server_addr.sin_family = AF_INET;

    // 3. Bind the server socket to port number
    if(bind(servSockFd,(struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        printErrorMsg("ERROR in binding server to given port number \n");
    }

    // 4. Listen to new incoming requests
    listen(servSockFd,5);
    cltlen = sizeof(client_addr);

    //5. Accept and create a new socket fpr client and assign a thread to new connection
    while((clientSockFd = accept(servSockFd,(struct sockaddr *)&client_addr,&cltlen)) > 0)
    {
        int *newSock;
        newSock = (int *)malloc(1*sizeof(int));
        *newSock = clientSockFd;
        pthread_t client;
        if(pthread_create(&client, NULL,client_handler,(void *)newSock) < 0)
        {
            printErrorMsg("ERROR in assiging a thread to new connection\n");
        }
        
    }
    if(clientSockFd < 0)
    {
        printErrorMsg("ERROR in creating a socket for client\n");
    }
    
    close(servSockFd);
    return 0;
}

void printErrorMsg(const char *msg){
    perror(msg);
    exit(0);
}

void *client_handler(void *clt){
    int clientSockFd = *((int *)clt),s;
    printf("Assigned a thread\n");
    // 6. Read and write from and to client
    char msgBuffer[256];
    bzero(msgBuffer,256);

    while((s = read(clientSockFd,msgBuffer,256)) > 0)
    {
       
        printf("The message sent by the client is : %s\n",msgBuffer);
        
        s = write(clientSockFd,"I got a message from you\n",25);
        
        if(s < 0)
        {
            printErrorMsg("ERROR sending a message to client\n");
        }
        bzero(msgBuffer,256);
    }
    if(s < 0)
    {
        printErrorMsg("ERROR in reading message from client\n");
    }
    else
    {
        printf("Client has disconnected\n");
    }

    close(clientSockFd);
    free(clt);
    return (void *)"";

}