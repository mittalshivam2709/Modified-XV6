#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>

int main(int argc, char **argv){

  // if (argc != 3){
  //   printf("Usage: %s <port>\n", argv[0]);
  //   exit(0);
  // }

  char *ip = "127.0.0.1";
  int port1 = 5566;
  int port2 = 5567;

  int sockfd1,sockfd2;
  struct sockaddr_in server_addr1,server_addr2, client_addr1,client_addr2;
  char bf1[1024];
  char bf2[1024];
  socklen_t addr_size1,addr_size2;
  // int n;

  sockfd1 = socket(AF_INET, SOCK_DGRAM, 0);
  sockfd2 = socket(AF_INET, SOCK_DGRAM, 0);
  if (sockfd1 < 0){
    perror("[-]socket error");
    exit(1);
  }
  if (sockfd2 < 0){
    perror("[-]socket error");
    exit(1);
  }
  printf("[+]UDP server socket1 and socket2 created.\n");

  memset(&server_addr1, '\0', sizeof(server_addr1));
  memset(&server_addr2, '\0', sizeof(server_addr2));
  server_addr1.sin_family = AF_INET;
  server_addr1.sin_port = htons(port1);
  server_addr1.sin_addr.s_addr = inet_addr(ip);
  server_addr2.sin_family = AF_INET;
  server_addr2.sin_port = htons(port2);
  server_addr2.sin_addr.s_addr = inet_addr(ip);

  int n1 = bind(sockfd1, (struct sockaddr*)&server_addr1, sizeof(server_addr1));
  int n2 = bind(sockfd2, (struct sockaddr*)&server_addr2, sizeof(server_addr2));
  if (n1 < 0) {
    perror("[-]bind error");
    exit(1);
  }
  if (n2 < 0) {
    perror("[-]bind error");
    exit(1);
  }
  printf("[+]Bind to the port number: 5566\n");
  printf("[+]Bind to the port number: 5567\n");

  addr_size1 = sizeof(client_addr1);
  addr_size2 = sizeof(client_addr2);

  while(1){
    bzero(bf1, 1024);
    bzero(bf2, 1024);
    // recvfrom(sockfd1, bf1, 1024, 0, (struct sockaddr*)&client_addr1, &addr_size1);
    // recvfrom(sockfd2, bf2, 1024, 0, (struct sockaddr*)&client_addr2, &addr_size2);
    int recvstatus=recvfrom(sockfd1, bf1, 1024, 0, (struct sockaddr*)&client_addr1, &addr_size1);
    if(recvstatus==-1){
      printf("unable to receive message\n");
      exit(1);
    }
    recvstatus=recvfrom(sockfd2, bf2, 1024, 0, (struct sockaddr*)&client_addr2, &addr_size2);
    if(recvstatus==-1){
      printf("unable to receive message\n");
      exit(1);
    }
    // break;
      int buffer1=atoi(bf1);
      int buffer2=atoi(bf2);
      printf("recieved %d %d\n",buffer1,buffer2);
          if(buffer1==0){
        if(buffer2==1){
          buffer1=-1;
          buffer2=1;
        }
        else if(buffer2==2){
          buffer1=1;
          buffer2=-1;
        }
        else if (buffer2==0){
          buffer1=buffer2=0;
        }   
        else{
          printf("invalid inputs by the clients\n");
          buffer1=-2;buffer2=-2;

          break;
        }
      }
      else if(buffer1==1){
        if(buffer2==0){
          buffer1=1;
          buffer2=-1;
        }
        else if(buffer2==2){
          buffer1=-1;
          buffer2=1;
        }
        else if (buffer2==1){
          buffer1=buffer2=0;
        }    
        else{
          printf("invalid inputs by the clients\n");
          buffer1=-2;buffer2=-2;
          // break;
        }  
      }
      else if(buffer1==2){
        if(buffer2==0){
          buffer2=1;
          buffer1=-1;
        }
        else if(buffer2==1){
          buffer1=1;
          buffer2=-1;
        }
        else if (buffer2==0){
          buffer1=buffer2=0;
        }
        else{
          printf("invalid inputs by the clients\n");
          buffer1=-2;buffer2=-2;
          break;
        }  
      }
      else{
        printf("invalid inputs by the clients\n");
          buffer1=-2;buffer2=-2;
        // break;
      }  
    char send1[1024],send2[1024];
    if(buffer1==-1){
      strcpy(send1,"LOST");
    }
    else if(buffer1==0){
      // send1='0';
      strcpy(send1,"DRAW");
    }
    else if(buffer1==1){
      // send1='1';
      strcpy(send1,"WIN");
    }
    else{
      strcpy(send1,"invalidinput");
      strcpy(send2,"invalidinput");
    }

    if(buffer2==-1){
      strcpy(send2,"LOST");
    }
    else if(buffer2==0){
      // send1='0';
      strcpy(send2,"DRAW");
    }
    else if(buffer2==1){
      // send1='1';
      strcpy(send2,"WIN");
    }
      printf("Server1: %s\n", send1);
      printf("Server2: %s\n", send2);
      // send(client_sock1, send1, strlen(send1), 0);

    // sendto(sockfd1, send1, 1024, 0, (struct sockaddr*)&client_addr1, sizeof(client_addr1));
    int sendstatus=sendto(sockfd1,send1 , 1024, 0, (struct sockaddr*)&client_addr1, sizeof(client_addr1));
    if(sendstatus==-1){
      printf("unable to send message\n");
      exit(1);
    }
    sendstatus=sendto(sockfd2,send2 , 1024, 0, (struct sockaddr*)&client_addr2, sizeof(client_addr2));
    if(sendstatus==-1){
      printf("unable to send message\n");
      exit(1);
    }
    // sendto(sockfd2, send2, 1024, 0, (struct sockaddr*)&client_addr2, sizeof(client_addr2));
    bzero(bf1, 1024);
    bzero(bf2, 1024);
    // recvfrom(sockfd1, bf1, 1024, 0, (struct sockaddr*)&client_addr1, &addr_size1);
    // recvfrom(sockfd2, bf2, 1024, 0, (struct sockaddr*)&client_addr2, &addr_size2);
    recvstatus=recvfrom(sockfd1, bf1, 1024, 0, (struct sockaddr*)&client_addr1, &addr_size1);
    if(recvstatus==-1){
      printf("unable to receive message\n");
      exit(1);
    }
    recvstatus=recvfrom(sockfd2, bf2, 1024, 0, (struct sockaddr*)&client_addr2, &addr_size2);
    if(recvstatus==-1){
      printf("unable to receive message\n");
      exit(1);
    }
    if(strcmp(bf1,bf2)==0 && strcmp(bf1,"yes")==0){
      // sendto(sockfd1, send1, 1024, 0, (struct sockaddr*)&client_addr1, sizeof(client_addr1));
      // sendto(sockfd2, send2, 1024, 0, (struct sockaddr*)&client_addr2, sizeof(client_addr2));
      int sendstatus=sendto(sockfd1,send1 , 1024, 0, (struct sockaddr*)&client_addr1, sizeof(client_addr1));
      if(sendstatus==-1){
        printf("unable to send message\n");
        exit(1);
      }
      sendstatus=sendto(sockfd2,send2 , 1024, 0, (struct sockaddr*)&client_addr2, sizeof(client_addr2));
      if(sendstatus==-1){
        printf("unable to send message\n");
        exit(1);
      }
    
      continue;
    }else{
      bzero(send1,1024);
      bzero(send2,1024);
      strcpy(send1,"break");
      strcpy(send2,"break");
      // sendto(sockfd1, send1, 1024, 0, (struct sockaddr*)&client_addr1, sizeof(client_addr1));
      // sendto(sockfd2, send2, 1024, 0, (struct sockaddr*)&client_addr2, sizeof(client_addr2));
      int sendstatus=sendto(sockfd1,send1 , 1024, 0, (struct sockaddr*)&client_addr1, sizeof(client_addr1));
      if(sendstatus==-1){
        printf("unable to send message\n");
        exit(1);
      }
      sendstatus=sendto(sockfd2,send2 , 1024, 0, (struct sockaddr*)&client_addr2, sizeof(client_addr2));
      if(sendstatus==-1){
        printf("unable to send message\n");
        exit(1);
      }
      break;
    }
  }
  printf("Disconnected !! \n");

  return 0;
}