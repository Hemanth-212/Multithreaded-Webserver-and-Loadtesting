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


queue<int>client_queue;
pthread_mutex_t queue_lock  = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t  cv = PTHREAD_COND_INITIALIZER;
bool done_work = true;

/*Prints error message*/
void printErrorMsg(const char *msg){
    perror(msg);
    exit(0);
}

/*Handles the client's request*/
void client_handler(int clt){

    int clientSockFd = clt,s;
    printf("Assigned a thread\n");

    char msgBuffer[256];
    bzero(msgBuffer,256);
    if((s = read(clientSockFd,msgBuffer,256)) > 0)
    {
        string request((char *)msgBuffer);
        HTTP_Response *http_response = handle_request(request);
        string http_response_str = http_response->get_string();
        char res[1000];
        strcpy(res,http_response_str.c_str());
        //printf("%s\n",res);
        s = write(clientSockFd,res,strlen(res));
        delete(http_response);
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
    return;

}

/*Function to start and handle the worker thread*/
void *start_worker_thread(void *arg){

    int clientSockFd;
    printf("Successfully created worker thread\n");
    
    while(1)
    {
        pthread_mutex_lock(&queue_lock);
        while(client_queue.size() == 0)
        {
            pthread_cond_wait(&cv,&queue_lock);
        }
        clientSockFd = client_queue.front();
        client_queue.pop();
        pthread_mutex_unlock(&queue_lock);
        client_handler(clientSockFd);

    }

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

     /* Create 10 worker threads */
    for(int t=0;t<10;++t)
    {
        pthread_t clt;
        if(pthread_create(&clt, NULL, start_worker_thread,NULL) < 0)
        {
            printErrorMsg("ERROR in creating a new worker thread\n");
        }
        
    }

    listen(servSockFd,20);
    cltlen = sizeof(client_addr);

    //5. Accept and create a new socket for client and add the socketfd into queue and then signal a worker thread
    while((clientSockFd = accept(servSockFd,(struct sockaddr *)&client_addr,&cltlen)) > 0)
    {
        pthread_mutex_lock(&queue_lock);
        client_queue.push(clientSockFd);
        pthread_cond_signal(&cv);
        pthread_mutex_unlock(&queue_lock);
    }
    if(clientSockFd < 0)
    {
        printErrorMsg("ERROR in creating a socket for client\n");

    }
    pthread_mutex_destroy(&queue_lock);
    close(servSockFd);
    
    return 0;
}