#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/select.h>

struct chunk
{
  char arr[12];
  int idx;
  int total;  
  int length;
};

int main(int argc, char **argv){
  char *ip = "127.0.0.1";
  int port = 5561;

  int sockfd;
  struct sockaddr_in server_addr, client_addr;
  char bf[1024];
  socklen_t addr_size;
  // int n;

  sockfd = socket(AF_INET, SOCK_DGRAM, 0);
  if (sockfd < 0){
    perror("[-]socket error");
    exit(1);
  }
  printf("[+]UDP server socket1 and socket2 created.\n");

  memset(&server_addr, '\0', sizeof(server_addr));
  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(port);
  server_addr.sin_addr.s_addr = inet_addr(ip);
  
  int n = bind(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr));
  if (n < 0) {
    perror("[-]bind error");
    exit(1);
  }
  printf("[+]Bind to the port number: 5561\n");
  addr_size = sizeof(client_addr);
  struct chunk chunkarr[10000];
  int i=0;
  int *foundarr;
  int total=0;
  int count=0;
  int f=0;
  while(1){
    struct chunk temp;
    recvfrom(sockfd, &temp, sizeof(struct chunk), 0, (struct sockaddr*)&client_addr, &addr_size);
    i++;
    // printf("here");
    if(i==1){
      // printf("ni");
      foundarr=(int*)malloc(sizeof(int)*(temp.total));
      total=temp.total;
    }
    // printf("\n%s %d\n",temp.arr,temp.length);
    // printf("%d ",total);
    foundarr[temp.idx]=1;
    chunkarr[temp.idx]=temp;
    // if(count%3==0){
      // printf("isider");
      if(f==1){
        struct chunk finalack;
        finalack.idx=total;
        sendto(sockfd, &finalack, sizeof(struct chunk), 0, (struct sockaddr*)&client_addr, sizeof(client_addr));
      }
      else{
        sendto(sockfd, &temp, sizeof(struct chunk), 0, (struct sockaddr*)&client_addr, sizeof(client_addr));
      }
    // }
    count++;
    f=0;
    // printf("%d ",temp.total);
    for (int j = 0; j < temp.total; j++){
      if(foundarr[j]==0){
        f++;
      }
    }
    if(f==0){
      break;
    }
  }
  for (int i = 0; i < total; i++){
    for (int j = 0; j <= chunkarr[i].length; j++){
      printf("%c",chunkarr[i].arr[j]);
    }
  }
  printf("\nDisconnected !! \n");
  return 0;
}