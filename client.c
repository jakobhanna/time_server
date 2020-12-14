#define _POSIX_C_SOURCE 200809L

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>

#include "message.h"

void err(char *msg)
{
  perror(msg);
  exit(EXIT_FAILURE);
}

// create a new socket, protocoll is tcp
int createSocket()
{
  //create client socket
  int soc = socket(PF_INET,SOCK_STREAM,0);
  if  (soc <0) {
    err("client socket");
  }
  printf("client socket created\n");

  return soc;
}

// connect given socket soc to server at "localhost" on port "2342"
void connectToTimeServer(int soc)
{ 
  // specify information about the hostname and port to use
  struct addrinfo hints;
  struct addrinfo *res;

  memset(&hints, 0, sizeof(hints)); // clear memory
  hints.ai_family = AF_INET;        // use IP as protocol
  hints.ai_socktype = SOCK_STREAM;  // use TCP as protocol

  // turn hostname and port information in addrinfo struct
  // information in hints is used as a hint to fill info
  int retval = getaddrinfo("localhost", "2342", &hints, &res);
  if (retval != 0) 
  {
    printf("getaddrinfo failed %s\n", gai_strerror(retval));
    exit(EXIT_FAILURE);
  }
  
  // connect socket to address
  retval = connect(soc, res->ai_addr, res->ai_addrlen);
  // release memory used by info
  freeaddrinfo(res);
  if (retval < 0) 
  { 
    {
    close(soc); 
    err("connect failed");
    exit(0);
    }
  printf("connected to localhost:2342\n");
  }
}

// send time request message to the server
void sendTimeRequest(int soc, const time_request_t* timeRequest)
{
  int bytessend = send(soc, (const void *)timeRequest, 
  sizeof(time_request_t), 0);
  if (bytessend < 0) { 
    close(soc); 
    err("error while sending"); 
    }
  printf("send %d bytes\n", bytessend);
}

// receive time respond message from the server and store it in timeRespond
void receiveTimeRespond(int soc, time_respond_t* timeRespond)
{
  // receive data on the connection socket and store them in timeRequest
  int bytesrcvd = recv(soc, (void *)timeRespond, 
  sizeof(time_respond_t), 0);
  if (bytesrcvd < 0) { 
    close(soc); 
    err("error while receiving"); 
    }
  printf("received %d bytes\n", bytesrcvd);
}

int main(void)
{
  int soc = createSocket();
  connectToTimeServer(soc);

  time_request_t timeRequest;
  time_respond_t timeRespond;

  timeRequest.timezone = +1; // CET
  sendTimeRequest(soc, &timeRequest);

  receiveTimeRespond(soc, &timeRespond);
  printf("Current time in Münster: %s", timeRespond.time);


  timeRequest.timezone = -5; // EST
  sendTimeRequest(soc, &timeRequest);

  receiveTimeRespond(soc, &timeRespond);
  printf("Current time in New York: %s", timeRespond.time);


  // send disconnect message
  timeRequest.timezone = 127; // magic number
  sendTimeRequest(soc, &timeRequest);
}
