#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>

int main(int argc, char **argv){

  char *ip = "127.0.0.1";
  int port = 5566;

  int sockfd;
  struct sockaddr_in addr;
  char buffer[1024];
  socklen_t addr_size;

  sockfd = socket(AF_INET, SOCK_DGRAM, 0);
  if (sockfd < 0){
    perror("[-]Socket error");
    exit(1);
  }
  printf("[+]UDP server socket created.\n");
  memset(&addr, '\0', sizeof(addr));
  addr.sin_family = AF_INET;
  addr.sin_port = htons(port);
  addr.sin_addr.s_addr = inet_addr(ip);

  while(1){
    char rps[1024];
    char output[1024];
    scanf("%s",rps);
    // sendto(sockfd, rps, 1024, 0, (struct sockaddr*)&addr, sizeof(addr));
    int sendstatus=sendto(sockfd, rps, 1024, 0, (struct sockaddr*)&addr, sizeof(addr));
    if(sendstatus==-1){
      printf("unable to send message\n");
      exit(1);
    }
    // send(sock, rps, strlen(rps), 0);
    bzero(output, 1024);
    
    // recv(sock, output, sizeof(output), 0);
    addr_size = sizeof(addr);
    // recvfrom(sockfd, output, 1024, 0, (struct sockaddr*)&addr, &addr_size);

    int receivestatus=recvfrom(sockfd, output, 1024, 0, (struct sockaddr*)&addr, &addr_size);
    if(receivestatus==-1){
      printf("unable to receive message\n");
      exit(1);
    }
    printf("%s\n",output);
    printf("do you want to continue playing ?\n");
    char yesno[1024];
    scanf("%s",yesno);
    // send(sock,yesno,strlen(yesno),0);
    // sendto(sockfd, yesno, 1024, 0, (struct sockaddr*)&addr, sizeof(addr));
    sendstatus=sendto(sockfd, yesno, 1024, 0, (struct sockaddr*)&addr, sizeof(addr));
    if(sendstatus==-1){
      printf("unable to send message\n");
      exit(1);
    }
    

    bzero(output,1024);
    // recv(sock,output,sizeof(output),0);
    // recvfrom(sockfd, output, 1024, 0, (struct sockaddr*)&addr, &addr_size);
    receivestatus=recvfrom(sockfd, output, 1024, 0, (struct sockaddr*)&addr, &addr_size);
    if(receivestatus==-1){
      printf("unable to receive message\n");
      exit(1);
    }

    if(strcmp(output,"break")==0){
      break;
    }
  }
  printf("Disconnected!!\n");
  return 0;
}