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
  struct sockaddr_in addr;
  char bf1[1204];
  // bf1=(char*)malloc(sizeof(char)*1024);
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
  gets(bf1);


  struct chunk chunkarray[((strlen(bf1)+11)/12)];
  int idx=0;

  for (int i = 0; i < ((strlen(bf1)+11)/12); i++){
    chunkarray[i].idx=i;
    chunkarray[i].total=((strlen(bf1)+11)/12);
    for (int j = 0; j < 12 && idx < strlen(bf1); j++){
      chunkarray[i].arr[j]=bf1[idx];
      chunkarray[i].length=j;
      idx++;
    }
  }
  // for (int i = 0; i < (strlen(bf1)+11)/12; i++){
  //   // for (int j = 0; j <= chunkarray[i].length; j++){
  //     printf("%d\n",chunkarray[i].length);
  //   // }
  // }

  // printf("%d ",strlen);
  int i=0;
  int ack[(strlen(bf1)+11)/12];
  for (int i = 0; i < ((strlen(bf1)+11)/12); i++){
    ack[i]=0;
  }
  int p=1;
  fd_set readfds;
  // printf("%d ",strlen(bf1));
  FD_ZERO(&readfds);
  FD_SET(sockfd, &readfds);
  addr_size = sizeof(addr); // Initialize addr_size

  while(1){
    // printf("in");
    // printf("%d ",chunkarray[i].total);
    if(ack[i]==0){
      sendto(sockfd, &chunkarray[i], sizeof(struct chunk), 0, (struct sockaddr*)&addr, sizeof(addr));
    }
    i++;
    if(i==(strlen(bf1)+11/12)){
      i=0;
    }
    fd_set tmp_fds = readfds;
    int f=0;
    // printf("here");    
    // select(FD_SETSIZE,&tmp_fds,NULL,NULL,NULL);
    struct timeval t;
    t.tv_sec=0;
    t.tv_usec=100000;
    if (select(FD_SETSIZE, &tmp_fds, NULL, NULL, &t) < 0) {
        perror("Select error");
        exit(EXIT_FAILURE);
    }
    if (FD_ISSET(sockfd, &tmp_fds)) {
      struct chunk temp;
      addr_size = sizeof(addr); // Initialize addr_size
      recvfrom(sockfd, &temp, sizeof(struct chunk), 0, (struct sockaddr*)&addr, &addr_size);
      if(temp.idx==((strlen(bf1) + 11) / 12)){
        break;
      }
      if (temp.idx >= ((strlen(bf1) + 11) / 12)) {
          continue;
      }
      ack[temp.idx] = 1;
    }
    for (int j = 0; j < ((strlen(bf1)+11)/12); j++){
      if(ack[j]==0){
        f=1;
        break;
      }
    }
    if(f==0){
      break;
    }
  }
  printf("Disconnected!!\n");
  return 0;
}