#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

int main(){

  char *ip = "127.0.0.1";
  int port = 3456;

  int sock;
  struct sockaddr_in addr;
  socklen_t addr_size;
  char buffer[1024];
  int n;

  sock = socket(AF_INET, SOCK_STREAM, 0);
  if (sock < 0){
    perror("[-]Socket error");
    exit(1);
  }
  printf("[+]TCP server socket created.\n");

  memset(&addr, '\0', sizeof(addr));
  addr.sin_family = AF_INET;
  addr.sin_port = port;
  addr.sin_addr.s_addr = inet_addr(ip);

  // connect(sock, (struct sockaddr*)&addr, sizeof(addr));
  int connectstatus = connect(sock, (struct sockaddr *)&addr, sizeof(addr));
  if (connectstatus == -1)
  {
      printf("conncetion failed\n");
      exit(1);
  }
  printf("Connected to the server.\n");

  while(1){
    char rps[1024];
    char output[1024];
    scanf("%s",rps);
    // send(sock, rps, strlen(rps), 0);
    long sendstatus=send(sock, rps, strlen(rps), 0);
    if(sendstatus==-1){
        printf("unable to send the message\n");
        exit(1);
    }
    bzero(output, 1024);
    
    // recv(sock, output, sizeof(output), 0);
    long recvstatus = recv(sock, output, sizeof(output), 0);
    if(recvstatus==-1){
      printf("unable to receive the message\n");
      exit(1);
    }
    // if(strcmp(output,"DRAW")==0 ||strcmp(output,"LOST")==0 || strcmp(output,"WIN")==0 ){
      printf("%s\n",output);
    // }
    printf("do you want to continue playing ?\n");
    char yesno[1024];
    scanf("%s",yesno);
    sendstatus= send(sock,yesno,strlen(yesno),0);
    if(sendstatus==-1){
      printf("unable to send the message\n");
      exit(1);
    }
    bzero(output,1024);
    recvstatus= recv(sock,output,sizeof(output),0);
    if(recvstatus==-1){
      printf("unable to receive the message\n");
      exit(1);
    }
    if(strcmp(output,"break")==0){
      break;
    }
  }
  close(sock);
  printf("Disconnected from the server.\n");
  return 0;
}