#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <netdb.h>

#include <pthread.h>
#include <sys/time.h>
#include<iostream>

using namespace std;
int time_up;
FILE *log_file;

// user info struct
struct user_info {
  // user id
  int id;

  // socket info
  int portno;
  char *hostname;
  float think_time;

  // user metrics
  int total_count;
  float total_rtt;
};

// error handling function
void printErrorMsg(const char *errMsg) {
  perror(errMsg);
  exit(0);
}

// time diff in seconds
float time_diff(struct timeval *t2, struct timeval *t1) {
  return (t2->tv_sec - t1->tv_sec) + (t2->tv_usec - t1->tv_usec) / 1e6;
}

// user thread function
void *user_function(void *arg) {
  /* get user info */
  struct user_info *info = (struct user_info *)arg;

  int clientSockfd, n,portNo;
  char msgBuffer[512];
  struct timeval start, end;

  struct sockaddr_in server_addr;
  struct hostent *serverent;

  while (1) {
    /* start timer */
    gettimeofday(&start, NULL);

    /* 1. Creating a client socket*/
    clientSockfd = socket(AF_INET,SOCK_STREAM,0);

    /* 2. Assigning Server attributes */
    serverent = gethostbyname(info->hostname);
    if(!serverent)
    {
      printErrorMsg("Can not find any such host\n");
    }

    portNo = htons(info->portno);
    bzero((char *)&server_addr,sizeof(server_addr));
    bcopy((char *)serverent->h_addr,(char *)&server_addr.sin_addr.s_addr,serverent->h_length);
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = portNo;

    /* 3. Connecting to Server */
    if(connect(clientSockfd,(struct sockaddr*)&server_addr,sizeof(server_addr)) < 0)
    {
      printErrorMsg("ERROR connecting to server\n");
    }

    /* 4. Sending message to server */
    int index = rand()%5050;
    string http_request = "GET /index"+to_string(index)+".html HTTP/1.0\nHost: localhost:8080\nAccept: */*\n";
    char request[256];
    strcpy(request,http_request.c_str());
    int send_request = write(clientSockfd,request,strlen(request));
    if(send_request < 0)
    {
      printErrorMsg("ERROR in sending request to server\n");
    }

    /* 5. Reading response form server */
    int read_response = read(clientSockfd,msgBuffer,512);
    if( read_response < 0 )
    {
      printErrorMsg("ERROR reading message from server");
    }
    //printf("Server message is : %s\n",msgBuffer);
    bzero(msgBuffer,512);
    
    close(clientSockfd);

    /* end timer */
    gettimeofday(&end, NULL);

    /* if time up, break */
    if (time_up)
      break;

    /* 6. Updating the user metrics of a thread*/
    info->total_count += 1;
    info->total_rtt  += time_diff(&end,&start);

    /* 7. Sleep for some think time*/
    //printf("%lf\n",info->think_time);
    usleep((info->think_time)*1000000);

  }

  /* exit thread */
  fprintf(log_file, "User #%d finished\n", info->id);
  fflush(log_file);
  pthread_exit(NULL);
}

int main(int argc, char *argv[]) {
  int user_count, portno, test_duration,i;
  float think_time;
  char *hostname;

  if (argc != 6) {
    fprintf(stderr,
            "Usage: %s <hostname> <server port> <number of concurrent users> "
            "<think time (in s)> <test duration (in s)>\n",
            argv[0]);
    exit(0);
  }

  hostname = argv[1];
  portno = atoi(argv[2]);
  user_count = atoi(argv[3]);
  think_time = atof(argv[4]);
  test_duration = atoi(argv[5]);

  printf("Hostname: %s\n", hostname);
  printf("Port: %d\n", portno);
  printf("User Count: %d\n", user_count);
  printf("Think Time: %f s\n", think_time);
  printf("Test Duration: %d s\n", test_duration);

  /* open log file */
  log_file = fopen("load_gen.log", "w");

  pthread_t threads[user_count];
  struct user_info info[user_count];
  struct timeval start, end;

  /* start timer */
  gettimeofday(&start, NULL);
  time_up = 0;
  for (i = 0; i < user_count; ++i) {

    /* Initializing each threads/clients user info data*/
    struct user_info temp_user;
    temp_user.id = i;
    temp_user.portno = portno;
    temp_user.hostname = hostname;
    temp_user.think_time = think_time;
    temp_user.total_count = 0;
    temp_user.total_rtt = 0;

    info[i] = temp_user;

    /* Creating user_count threads */
    if(pthread_create(&threads[i],NULL,user_function,(void *)&info[i]) < 0)
    {
      printErrorMsg("ERROR in creating a thread for a user\n");
    }
    fprintf(log_file, "Created thread %d\n", i);
  }

  /* Main threads sleeps for test_duration time */
  usleep(test_duration*1000000);
  fprintf(log_file, "Woke up\n");

  /* end timer */
  time_up = 1;
  gettimeofday(&end, NULL);

  /* Wait until all threads complete execution*/
  for(i=0;i<user_count;++i)
  {
    pthread_join(threads[i],NULL);
  }

  /*Summing up all the thread's total request handled and also total reponse time*/
  int total = 0;
  double total_rtt = 0;
  for(i=0;i<user_count;++i)
  {
    total += info[i].total_count;
    total_rtt += info[i].total_rtt;
  }

  double total_runtime = time_diff(&end,&start);

  printf("The total no of requests handled in %d time duration is : %d\n",test_duration,total);
  printf("The throughput of the server is : %d \n",(int)(total/test_duration));
  printf("The response time for all process is : %lf  \n",total_rtt/total);
  /* close log file */
  fclose(log_file);

  return 0;
}
