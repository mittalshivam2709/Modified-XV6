// #include <stdio.h>
// #include <stdlib.h>
// #include <string.h>
// #include <unistd.h>
// #include <arpa/inet.h>

// int main(){

//   char *ip = "127.0.0.1";
//   int port1 = 5566;
//   int port2 = 5567;

//   int server_sock1,server_sock2, client_sock1,client_sock2;
//   struct sockaddr_in server_addr1,server_addr2, client_addr1,client_addr2;
//   socklen_t addr_size1,addr_size2;
//   char bf1[1024];
//   char bf2[1024];

//   server_sock1 = socket(AF_INET, SOCK_STREAM, 0);
//   server_sock2 = socket(AF_INET, SOCK_STREAM, 0);
//   if (server_sock1 < 0){
//     perror("[-]Socket error");
//     exit(1);
//   }
//   if (server_sock2 < 0){
//     perror("[-]Socket error");
//     exit(1);
//   }
//   printf("[+]TCP server socket1 and socket2 created.\n");

//   memset(&server_addr2, '\0', sizeof(server_addr2));
//   memset(&server_addr1, '\0', sizeof(server_addr1));
//   server_addr1.sin_family = AF_INET;
//   server_addr1.sin_port = port1;
//   server_addr1.sin_addr.s_addr = inet_addr(ip);
//   server_addr2.sin_family = AF_INET;
//   server_addr2.sin_port = port2;
//   server_addr2.sin_addr.s_addr = inet_addr(ip);

//   int n1 = bind(server_sock1, (struct sockaddr*)&server_addr1, sizeof(server_addr1));
//   int n2 = bind(server_sock2, (struct sockaddr*)&server_addr2, sizeof(server_addr2));
//   if (n1 < 0){
//     perror("[-]Bind error");
//     exit(1);
//   }
//   if (n2 < 0){
//     perror("[-]Bind error");
//     exit(1);
//   }
//   printf("[+]Bind to the port number: %d\n", port1);
//   printf("[+]Bind to the port number: %d\n", port2);

//   int listenflag1=listen(server_sock1, 5);
//   int listenflag2=listen(server_sock2, 5);
//   if(listenflag1==-1){
//     printf("listen function failed\n");
//     exit(1);
//   }
//   if(listenflag2==-1){
//     printf("listen function failed\n");
//     exit(1);
//   }
//   printf("Listening to both the ports ...\n");

//   addr_size1 = sizeof(client_addr1);
//   addr_size2 = sizeof(client_addr2);
//   client_sock1 = accept(server_sock1, (struct sockaddr*)&client_addr1, &addr_size1);
//   client_sock2 = accept(server_sock2, (struct sockaddr*)&client_addr2, &addr_size2);
//   if(client_sock1==-1){
//     printf("connection with client1 failed\n");
//     printf("retrying......\n");
//     while(client_sock1==-1){
//       client_sock1 = accept(server_sock1, (struct sockaddr*)&client_addr1, &addr_size1);
//     }
//   }
//   if(client_sock2==-1){
//     printf("connection with client2 failed\n");
//     printf("retrying......\n");
//     while(client_sock2==-1){
//       client_sock2 = accept(server_sock2, (struct sockaddr*)&client_addr2, &addr_size2);
//     }
//   }
//   printf("[+]Client1 and Client2 connected.\n");
//   while(1){
//     bzero(bf1, 1024);
//     bzero(bf2, 1024);
//     recv(client_sock1, bf1, sizeof(bf1), 0);
//     recv(client_sock2, bf2, sizeof(bf2), 0);
//     long recvstatus1=recv(client_sock1, bf1, sizeof(bf1), 0);
//     long recvstatus2=recv(client_sock2, bf2, sizeof(bf2), 0);
//     if(recvstatus1==-1){
//         printf("unable to receive message from client1 \n");
//         continue;
//     }
//     if(recvstatus2==-1){
//         printf("unable to receive message from client2 \n");
//         continue;
//     }
//     printf("Client1: %s\n", bf1);
//     printf("Client2: %s\n", bf2);

//     // bzero(buffer, 1024);
//     // strcpy(buffer, "HI, THIS IS SERVER. HAVE A NICE DAY!!!");
//     int buffer1=atoi(bf1);
//     int buffer2=atoi(bf2);
//     if(buffer1==0){
//       if(buffer2==1){
//         buffer1=-1;
//         buffer2=1;
//       }
//       else if(buffer2==2){
//         buffer1=1;
//         buffer2=-1;
//       }
//       else if (buffer2==0){
//         buffer1=buffer2=0;
//       }
//     }
//     else if(buffer1==1){
//       if(buffer2==0){
//         buffer1=1;
//         buffer2=-1;
//       }
//       else if(buffer2==2){
//         buffer1=-1;
//         buffer2=1;
//       }
//       else if (buffer2==1){
//         buffer1=buffer2=0;
//       }
//     }
//     else if(buffer1==2){
//       if(buffer2==0){
//         buffer2=1;
//         buffer1=-1;
//       }
//       else if(buffer2==1){
//         buffer1=1;
//         buffer2=-1;
//       }
//       else if (buffer2==0){
//         buffer1=buffer2=0;
//       }
//     }
//     else{
//       printf("invalid inputs by the clients\n");
//       continue;
//     }
//     char send1[1024],send2[1024];
//     if(buffer1==-1){
//       strcpy(send1,"LOST");
//     }
//     else if(buffer1==0){
//       // send1='0';
//       strcpy(send1,"DRAW");
//     }
//     else if(buffer1==1){
//       // send1='1';
//       strcpy(send1,"WIN");
//     }
//     if(buffer2==-1){
//       strcpy(send2,"LOST");
//     }
//     else if(buffer2==0){
//       // send1='0';
//       strcpy(send2,"DRAW");
//     }
//     else if(buffer2==1){
//       // send1='1';
//       strcpy(send2,"WIN");
//     }
//     printf("Server to client 1: %s\n", send1);
//     printf("Server to client 2: %s\n", send2);
//     int sendstatus1=send(client_sock1, send1, strlen(send1), 0);
//     if(sendstatus1==-1){
//       printf("unable to send message1\n");
//       printf("retrying.....\n");
//       while(sendstatus1==-1){
//         sendstatus1=send(client_sock1, send1, strlen(send1), 0);
//       }
//     }
//     int sendstatus2=send(client_sock2, send2, strlen(send2), 0);
//     if(sendstatus2==-1){
//       printf("unable to send message2\n");
//       printf("retrying.....\n");
//       while(sendstatus2==-1){
//         sendstatus2=send(client_sock2, send2, strlen(send2), 0);
//       }
//     }
//     bzero(bf1, 1024);
//     bzero(bf2, 1024);
//     long recvstatus11=recv(client_sock1, bf1, sizeof(bf1), 0);
//     if(recvstatus11==-1){
//       printf("unable to receive message 1\n ");
//       printf("retrying....\n");
//       while(recvstatus11==-1){
//         recvstatus11=recv(client_sock1, bf1, sizeof(bf1), 0);
//       }
//     }
//     long recvstatus22=recv(client_sock2, bf2, sizeof(bf2), 0);   
//     if(recvstatus22==-1){
//       printf("unable to receive message 2\n ");
//       printf("retrying....\n");
//       while(recvstatus22==-1){
//         recvstatus22=recv(client_sock2, bf2, sizeof(bf2), 0);   
//       }
//     }
//     if(strcmp(bf1,bf2)==0 && strcmp(bf1,"yes")==0){
//       printf("Both the clients said yes\n");
//       send(client_sock1, send1, strlen(send1), 0);
//       send(client_sock2, send2, strlen(send2), 0);
//       continue;
//     }else{
//       printf("atleast one client said no\n");
//       bzero(send1,1024);
//       bzero(send2,1024);
//       strcpy(send1,"break");
//       strcpy(send2,"break");
//       send(client_sock1, send1, strlen(send1), 0);
//       send(client_sock2, send2, strlen(send2), 0);
//       break;
//     }
//   }
//   close(client_sock1);
//   close(client_sock2);
//   printf("[+]Client disconnected.\n\n");
//   return 0;
// }

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

int main(){

  char *ip = "127.0.0.1";
  int port1 = 3456;
  int port2 = 4567;

  int server_sock1,server_sock2, client_sock1,client_sock2;
  struct sockaddr_in server_addr1,server_addr2, client_addr1,client_addr2;
  socklen_t addr_size1,addr_size2;
  char bf1[1024];
  char bf2[1024];

  server_sock1 = socket(AF_INET, SOCK_STREAM, 0);
  server_sock2 = socket(AF_INET, SOCK_STREAM, 0);
  if (server_sock1 < 0){
    perror("[-]Socket error");
    exit(1);
  }
  if (server_sock2 < 0){
    perror("[-]Socket error");
    exit(1);
  }
  printf("[+]TCP server socket1 and socket2 created.\n");

  memset(&server_addr2, '\0', sizeof(server_addr2));
  memset(&server_addr1, '\0', sizeof(server_addr1));
  server_addr1.sin_family = AF_INET;
  server_addr1.sin_port = port1;
  server_addr1.sin_addr.s_addr = inet_addr(ip);
  server_addr2.sin_family = AF_INET;
  server_addr2.sin_port = port2;
  server_addr2.sin_addr.s_addr = inet_addr(ip);

  int n1 = bind(server_sock1, (struct sockaddr*)&server_addr1, sizeof(server_addr1));
  int n2 = bind(server_sock2, (struct sockaddr*)&server_addr2, sizeof(server_addr2));
  if (n1 < 0){
    perror("[-]Bind error");
    exit(1);
  }
  if (n2 < 0){
    perror("[-]Bind error");
    exit(1);
  }
  printf("[+]Bind to the port number: %d\n", port1);
  printf("[+]Bind to the port number: %d\n", port2);

  int listenflag1=listen(server_sock1, 5);
  if(listenflag1==-1){
    printf("listen function failed\n");
    exit(1);
  }
  int listenflag2=listen(server_sock2, 5);
  if(listenflag2==-1){
    printf("listen function failed\n");
    exit(1);
  }
  printf("Listening...\n");

  addr_size1 = sizeof(client_addr1);
  addr_size2 = sizeof(client_addr2);
  client_sock1 = accept(server_sock1, (struct sockaddr*)&client_addr1, &addr_size1);
  client_sock2 = accept(server_sock2, (struct sockaddr*)&client_addr2, &addr_size2);
  if(client_sock1==-1){
    printf("connection with client failed\n");
    // continue;
    exit(1);
  }
  if(client_sock2==-1){
    printf("connection with client failed\n");
    // continue;
    exit(1);
  }
  printf("[+]Client1 and Client2 connected.\n");
  while(1){
    bzero(bf1, 1024);
    bzero(bf2, 1024);
    // recv(client_sock1, bf1, sizeof(bf1), 0);
    long recvstatus=recv(client_sock1, bf1, sizeof(bf1), 0);
    if(recvstatus==-1){
        printf("unable to receive message\n");
        continue;
    }
    // recv(client_sock2, bf2, sizeof(bf2), 0);
    recvstatus=recv(client_sock2, bf2, sizeof(bf2), 0);
    if(recvstatus==-1){
        printf("unable to receive message\n");
        continue;
    }
    printf("Client1: %s\n", bf1);
    printf("Client2: %s\n", bf2);

    // bzero(buffer, 1024);
    // strcpy(buffer, "HI, THIS IS SERVER. HAVE A NICE DAY!!!");
    int buffer1=atoi(bf1);
    int buffer2=atoi(bf2);
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
    long sendstatus=send(client_sock1, send1, strlen(send1), 0);
    if(sendstatus==-1){
        printf("unable to send message\n");
        continue;
    }
    // send(client_sock2, send2, strlen(send2), 0);
    sendstatus=send(client_sock2, send2, strlen(send2), 0);
    if(sendstatus==-1){
        printf("unable to send message\n");
        continue;
    }

    bzero(bf1, 1024);
    bzero(bf2, 1024);
    // recv(client_sock1, bf1, sizeof(bf1), 0);
    // recv(client_sock2, bf2, sizeof(bf2), 0);   

    recvstatus=recv(client_sock1, bf1, sizeof(bf1), 0);
    if(recvstatus==-1){
        printf("unable to receive message\n");
        continue;
    }
    // recv(client_sock2, bf2, sizeof(bf2), 0);
    recvstatus=recv(client_sock2, bf2, sizeof(bf2), 0);
    if(recvstatus==-1){
        printf("unable to receive message\n");
        continue;
    }
    if(strcmp(bf1,bf2)==0 && strcmp(bf1,"yes")==0){
      // send(client_sock1, send1, strlen(send1), 0);
      // send(client_sock2, send2, strlen(send2), 0);
      sendstatus=send(client_sock1, send1, strlen(send1), 0);
      if(sendstatus==-1){
          printf("unable to send message\n");
          continue;
      }
      // send(client_sock2, send2, strlen(send2), 0);
      sendstatus=send(client_sock2, bf2, strlen(bf2), 0);
      if(sendstatus==-1){
          printf("unable to send message\n");
          continue;
      }
      continue;
    }else{
      bzero(send1,1024);
      bzero(send2,1024);
      strcpy(send1,"break");
      strcpy(send2,"break");
      // send(client_sock1, send1, strlen(send1), 0);
      // send(client_sock2, send2, strlen(send2), 0);
      sendstatus=send(client_sock1, send1, strlen(send1), 0);
      if(sendstatus==-1){
          printf("unable to send message\n");
          continue;
      }
      // send(client_sock2, send2, strlen(send2), 0);
      sendstatus=send(client_sock2, bf2, strlen(bf2), 0);
      if(sendstatus==-1){
          printf("unable to send message\n");
          continue;
      }
      break;
    }
  }
  close(client_sock1);
  close(client_sock2);
  printf("[+]Client disconnected.\n\n");

  return 0;
}