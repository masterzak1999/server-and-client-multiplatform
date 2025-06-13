// server.c
#include "common.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <errno.h>

#define BACKLOG 10
#define BUF_SZ  4096

static volatile int terminate_flag = 0;
static int          dry_run       = 0;
static int          do_update     = 0;
static char        *auth_key      = NULL;
static socket_t     listener_fd;

static void logp(const char *fmt, ...) {
  va_list ap; va_start(ap, fmt);
  vprintf(fmt, ap); printf("\n");
  va_end(ap);
}
static int run_cmd(const char *cmd) {
  if (dry_run) { logp("[DRY] %s", cmd); return 0; }
  logp("[RUN] %s", cmd);
  return system(cmd);
}
static void handle_sig(int sig) {
  (void)sig; terminate_flag = 1;
}

static void handle_get(socket_t c, const char *path) {
  FILE *f = fopen(path,"rb");
  if (!f) {
    char e[]="ERROR: open\n";
    send_enc(c,e,strlen(e),auth_key,strlen(auth_key));
    return;
  }
  fseek(f,0,SEEK_END); long sz=ftell(f); fseek(f,0,SEEK_SET);
  char hdr[64]; snprintf(hdr,sizeof(hdr),"FILE %ld\n",sz);
  send_enc(c,hdr,strlen(hdr),auth_key,strlen(auth_key));
  unsigned char buf[BUF_SZ];
  size_t r;
  while((r=fread(buf,1,BUF_SZ,f))>0)
    send_enc(c,buf,r,auth_key,strlen(auth_key));
  fclose(f);
}

static void handle_put(socket_t c, const char *path) {
  char hdr[BUF_SZ];
  ssize_t n = recv_enc(c,hdr,sizeof(hdr),auth_key,strlen(auth_key));
  if (n<=0) return;
  long sz; if (sscanf(hdr,"PUTSIZE %ld",&sz)!=1) return;
  FILE *f = fopen(path,"wb"); if(!f) return;
  unsigned char buf[BUF_SZ]; long got=0;
  while(got<sz) {
    n = recv_enc(c,buf,BUF_SZ,auth_key,strlen(auth_key));
    if(n<=0) break;
    fwrite(buf,1,n,f); got+=n;
  }
  fclose(f);
  char ok[]="OK\n";
  send_enc(c,ok,strlen(ok),auth_key,strlen(auth_key));
}

static void *client_thread(void *arg) {
  socket_t fd = (socket_t)(uintptr_t)arg;
  char buf[BUF_SZ];
  ssize_t n;
  // auth
  n = recv_enc(fd,buf,BUF_SZ,auth_key,strlen(auth_key));
  if(n<=0) goto end;
  buf[n-1]=0;
  if(strcmp(buf,auth_key)!=0) {
    send_enc(fd,"AUTH_FAIL\n",10,auth_key,strlen(auth_key));
    goto end;
  }
  send_enc(fd,"OK\n",3,auth_key,strlen(auth_key));
  while(!terminate_flag &&
        (n=recv_enc(fd,buf,BUF_SZ,auth_key,strlen(auth_key)))>0) {
    buf[n-1]=0;
    if(strcmp(buf,"exit")==0) break;
    if(strncmp(buf,"GET ",4)==0) { handle_get(fd,buf+4); continue; }
    if(strncmp(buf,"PUT ",4)==0) { handle_put(fd,buf+4); continue; }
    FILE *p=popen(buf,"r");
    if(!p) {
      snprintf(buf,BUF_SZ,"ERROR: %s\n",strerror(errno));
      send_enc(fd,buf,strlen(buf),auth_key,strlen(auth_key));
      continue;
    }
    while(fgets(buf,BUF_SZ,p))
      send_enc(fd,buf,strlen(buf),auth_key,strlen(auth_key));
    pclose(p);
  }
end:
  close_socket(fd);
  return NULL;
}

int main(int argc,char **argv) {
  int port=12345, yes=1;
  thread_t tid;
  init_net();
  signal(SIGINT,handle_sig);
  signal(SIGTERM,handle_sig);
  for(int i=1;i<argc;i++){
    if(strcmp(argv[i],"-u")==0)      do_update=1;
    else if(strcmp(argv[i],"-d")==0) dry_run=1;
    else if(strcmp(argv[i],"-k")==0 && i+1<argc) auth_key=argv[++i];
    else if(strcmp(argv[i],"-p")==0 && i+1<argc) port=atoi(argv[++i]);
  }
  if(!auth_key){ fprintf(stderr,"-k TOKEN required\n"); return 1; }
  if(do_update){ run_cmd("apt-get update -y"); run_cmd("apt-get upgrade -y"); }
  struct addrinfo h={.ai_family=AF_UNSPEC,.ai_socktype=SOCK_STREAM,.ai_flags=AI_PASSIVE},*r;
  char ps[6]; snprintf(ps,6,"%d",port);
  getaddrinfo(NULL,ps,&h,&r);
  listener_fd=socket(r->ai_family,r->ai_socktype,0);
  setsockopt(listener_fd,SOL_SOCKET,SO_REUSEADDR,&yes,sizeof(yes));
  bind(listener_fd,r->ai_addr,r->ai_addrlen);
  listen(listener_fd,BACKLOG);
  freeaddrinfo(r);
  logp("Server listening on %d%s",port,dry_run?" (dry-run)":"");
  while(!terminate_flag){
    struct sockaddr_storage ss; socklen_t sl=sizeof(ss);
    socket_t c=accept(listener_fd,(struct sockaddr*)&ss,&sl);
    if(c<0) continue;
    thread_create(tid,client_thread,(void*)(uintptr_t)c);
  }
  logp("Shutting down");
  close_socket(listener_fd);
  cleanup_net();
  return 0;
}
