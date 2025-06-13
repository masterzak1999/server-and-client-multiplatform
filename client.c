// client.c
#include "common.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BUF_SZ 4096

static void usage(const char *p){
  fprintf(stderr,"Usage: %s -h HOST -p PORT -k TOKEN [-d]\n",p);
  exit(1);
}

int main(int argc,char **argv){
  char *host=NULL,*port=NULL,*key=NULL; int dry=0;
  for(int i=1;i<argc;i++){
    if(!strcmp(argv[i],"-h")&&i+1<argc) host=argv[++i];
    else if(!strcmp(argv[i],"-p")&&i+1<argc) port=argv[++i];
    else if(!strcmp(argv[i],"-k")&&i+1<argc) key=argv[++i];
    else if(!strcmp(argv[i],"-d")) dry=1;
  }
  if(!host||!port||!key) usage(argv[0]);
  init_net();
  socket_t s;
  if(!dry){
    struct addrinfo h={.ai_family=AF_UNSPEC,.ai_socktype=SOCK_STREAM},*r;
    getaddrinfo(host,port,&h,&r);
    s=socket(r->ai_family,r->ai_socktype,0);
    connect(s,r->ai_addr,r->ai_addrlen);
    freeaddrinfo(r);
    send_enc(s,key,strlen(key),key,strlen(key));
    send_enc(s,"\n",1,key,strlen(key));
    char ack[BUF_SZ];
    recv_enc(s,ack,BUF_SZ,key,strlen(key));
    if(strncmp(ack,"OK",2)) return printf("Auth failed\n"),1;
    printf("Connected to %s:%s\n",host,port);
  }
  char buf[BUF_SZ];
  while(fgets(buf,BUF_SZ,stdin)){
    buf[strcspn(buf,"\n")]=0;
    if(!strcmp(buf,"exit")) break;
    if(dry){ printf("[DRY] %s\n",buf); continue; }
    if(!strncasecmp(buf,"GET ",4)||!strncasecmp(buf,"PUT ",4)){
      send_enc(s,buf,strlen(buf),key,strlen(key));
      send_enc(s,"\n",1,key,strlen(key));
      continue;
    }
    send_enc(s,buf,strlen(buf),key,strlen(key));
    send_enc(s,"\n",1,key,strlen(key));
    ssize_t n;
    while((n=recv_enc(s,buf,BUF_SZ,key,strlen(key)))>0)
      printf("%.*s",(int)n,buf);
  }
  if(!dry) close_socket(s);
  cleanup_net();
  return 0;
}

// CMakeLists.txt
cmake_minimum_required(VERSION 3.5)
project(remote_cmd C)

add_executable(server server.c common.h)
add_executable(client client.c common.h)

if(WIN32)
  target_link_libraries(server PRIVATE ws2_32)
  target_link_libraries(client PRIVATE ws2_32)
endif()
