//
// Created by jack lewis on 11/18/25.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

struct client {
  // dimension, region, chunk, etc coords
};

struct server {
  // connected clients and associated info
  // currently loaded world data
  // properties and settings
};

// functionality: the ability to serialize world data to send to clients

void * serverThread(void * serverProperties) {
  int s=socket(AF_INET,SOCK_STREAM,0), c[64]={0}, i, j, n;
  fd_set f; char b[1024];
  struct sockaddr_in a={AF_INET,htons(5000),{0}};
  bind(s,(void*)&a,sizeof a);
  listen(s,8);fcntl(s,F_SETFL,O_NONBLOCK);
  for(;;){
    FD_ZERO(&f);
    FD_SET(s,&f);
    int m=s;
    for(i=0;i<64;i++)
      if(c[i])
        FD_SET(c[i],&f),
            m=c[i]>m?c[i]:m;
    if(select(m+1,&f,0,0,0)<1)
      continue;
    if(FD_ISSET(s,&f))
      for(i=0;i<64;i++)
        if(!c[i]){
      c[i]=accept(s,0,0);fcntl(c[i],F_SETFL,O_NONBLOCK);
      break;}
    for(i=0;i<64;i++)
      if(c[i]&&FD_ISSET(c[i],&f)){
        n=recv(c[i],b,sizeof b,0);
        if(n<=0){
          close(c[i]);c[i]=0;
          continue;}
      /* b[0] = msg type: 1=playerPos(12b) 2=voxelEdit(...) */
        for(j=0;j<64;j++)if(c[j]&&j!=i)send(c[j],b,n,0);
    }
  }
}



void * clientThread(void * arg) {
  struct client {
    int port;
    char * ip;
  };
  
  struct client * c = (struct client *) arg;
  
  int sock;
  
  int s = socket(AF_INET, SOCK_STREAM, 0);
  if(s < 0) { perror("socket"); exit(1); }

  struct sockaddr_in addr = {AF_INET, htons(c->port), {0}};
  if(inet_pton(AF_INET, c->ip, &addr.sin_addr) <= 0) { perror("inet_pton"); exit(1); }

  if(connect(s, (struct sockaddr*)&addr, sizeof(addr)) < 0) { perror("connect"); exit(1); }

  fcntl(s, F_SETFL, O_NONBLOCK); // nonblocking socket
  sock = s;
  fd_set f;
  char buf[1024];
  for(;;) {
    FD_ZERO(&f); FD_SET(s, &f); FD_SET(0, &f); // 0 = stdin
    int m = s > 0 ? s : 0;
    if(select(m+1, &f, NULL, NULL, NULL) > 0) {
      if(FD_ISSET(s, &f)) { // server sent data
        int n = recv(s, buf, sizeof(buf), 0);
        if(n <= 0) { printf("Disconnected\n"); break; }
        write(1, buf, n); // print to stdout
      }
      if(FD_ISSET(0, &f)) { // user typed data
        int n = read(0, buf, sizeof(buf));
        if(n > 0) send(s, buf, n, 0);
      }
    }
  }
  
  client_loop(sock);
  close(sock);
  return 0;
}
