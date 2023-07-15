#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<string.h>
#include<netdb.h>
#include<pthread.h>
#include "http_server.cpp"
#include<iostream>
#include<queue>

using namespace std;




/*Prints error message*/
void printErrorMsg(const char *msg){
    perror(msg);
    exit(0);
}

/*Handles the client's request*/
void *client_handler(void *clt){

    int clientSockFd = *((int*)clt),s;
    printf("Assigned a thread\n");

    char msgBuffer[256];
    bzero(msgBuffer,256);
    if((s = read(clientSockFd,msgBuffer,256)) > 0)
    {
        string request((char *)msgBuffer);
        HTTP_Response *http_response = handle_request(request);
        string http_response_str = http_response->get_string();
        char res[1000];
        for(int i=0;i<http_response_str.size();++i)
        {
            res[i]=http_response_str[i];
        }
        //cout<<http_response_str<<endl;
        s = write(clientSockFd,res,strlen(res));
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
    return (void *)"";

}



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
        int *newsock;
        newsock = (int *)malloc(1);
        *newsock = clientSockFd;
        pthread_t clt;
        if(pthread_create(&clt, NULL, client_handler,(void *)newsock) < 0)
        {
            printErrorMsg("ERROR in assigning a thread to new commection\n");
        }
        printf("Assigned a thread\n");
    }
    if(clientSockFd < 0)
    {
        printErrorMsg("ERROR in creating a socket for client\n");
    }
    
    close(servSockFd);
    return 0;
}




