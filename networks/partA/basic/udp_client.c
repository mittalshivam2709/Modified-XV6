#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>

int main(int argc, char **argv){

  char *ip = "127.0.0.1";
  int port = 1234;

  int sockfd;
  struct sockaddr_in addr;
  char buffer[1024];
  socklen_t addr_size;

  sockfd = socket(AF_INET, SOCK_DGRAM, 0);
  memset(&addr, '\0', sizeof(addr));
  addr.sin_family = AF_INET;
  addr.sin_port = htons(port);
  addr.sin_addr.s_addr = inet_addr(ip);

  bzero(buffer, 1024);
  strcpy(buffer, "Hello, World!");
  int sendstatus=sendto(sockfd, buffer, 1024, 0, (struct sockaddr*)&addr, sizeof(addr));
  if(sendstatus==-1){
    printf("unable to send message\n");
    exit(1);
  }
  printf("[+]Data send: %s\n", buffer);

  bzero(buffer, 1024);
  addr_size = sizeof(addr);
  int receivestatus=recvfrom(sockfd, buffer, 1024, 0, (struct sockaddr*)&addr, &addr_size);
  if(receivestatus==-1){
    printf("unable to receive message\n");
    exit(1);
  }

  printf("[+]Data recv: %s\n", buffer);

  return 0;
}