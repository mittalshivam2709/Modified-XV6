#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

int main(){
  char *ip = "127.0.0.1";
  int port = 2345;
  
  int server_sock, client_sock;
  struct sockaddr_in server_addr, client_addr;
  socklen_t addr_size;
  char buffer[1024];
  int n;

  server_sock = socket(AF_INET, SOCK_STREAM, 0);
  if (server_sock < 0){
    perror("[-]Socket error");
    exit(1);
  }
  printf("[+]TCP server socket created.\n");

  memset(&server_addr, '\0', sizeof(server_addr));
  server_addr.sin_family = AF_INET;
  server_addr.sin_port = port;
  server_addr.sin_addr.s_addr = inet_addr(ip);

  n = bind(server_sock, (struct sockaddr*)&server_addr, sizeof(server_addr));
  if (n < 0){
    perror("[-]Bind error");
    exit(1);
  }
  printf("[+]Bind to the port number: %d\n", port);

  int listenflag=listen(server_sock, 5);
  if(listenflag==-1){
    printf("listen function failed\n");
    exit(1);
  }
  printf("Listening...\n");

  while(1){
    addr_size = sizeof(client_addr);
    client_sock = accept(server_sock, (struct sockaddr*)&client_addr, &addr_size);
    if(client_sock==-1){
        printf("connection with client failed\n");
        continue;
    }
    printf("[+]Client connected.\n");

    bzero(buffer, 1024);
    long recvstatus=recv(client_sock, buffer, sizeof(buffer), 0);
    if(recvstatus==-1){
        printf("unable to receive message\n");
        continue;
    }
    printf("Client: %s\n", buffer);
    bzero(buffer, 1024);
    strcpy(buffer, "HI, THIS IS SERVER. HAVE A NICE DAY!!!");
    printf("Server: %s\n", buffer);
    long sendstatus=send(client_sock, buffer, strlen(buffer), 0);
    if(sendstatus==-1){
        printf("unable to send message\n");
        continue;
    }
    int closestatus=close(client_sock);
    if(closestatus==-1){
        printf("unable to disconnect to the client\n");
        printf("retrying.....\n");
        while(closestatus==-1){
            closestatus=close(client_sock);
        }
    }
    printf("[+]Client disconnected.\n\n");
    break;
  }
  return 0;
}