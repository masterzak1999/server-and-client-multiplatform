// server.c v3
// Remote-Cmd Server con auth token, syslog, graceful shutdown

#define _GNU_SOURCE
#include <arpa/inet.h>
#include <errno.h>
#include <getopt.h>
#include <netdb.h>
#include <pthread.h>
#include <signal.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>
#include <unistd.h>

#define BACKLOG   10
#define BUF_SZ    4096
#define VERSION   "v3.0.0"

static int  dry_run   = 0;
static int  do_update = 0;
static char *auth_key = NULL;
static int  sockfd    = -1;
static volatile sig_atomic_t terminate = 0;

// log su syslog e stdout
static void log_msg(int prio, const char *fmt, ...) {
    va_list ap; va_start(ap, fmt);
    vsyslog(prio, fmt, ap);
    va_end(ap);
    if (!dry_run) {
        va_start(ap, fmt);
        vfprintf(prio<=LOG_INFO?stdout:stderr, fmt, ap);
        fprintf(prio<=LOG_INFO?stdout:stderr, "\n");
        va_end(ap);
    }
}

// esegue o mostra il comando
static int run_cmd(const char *cmd) {
    if (dry_run) {
        log_msg(LOG_INFO, "[DRY] %s", cmd);
        return 0;
    }
    log_msg(LOG_INFO, "[RUN] %s", cmd);
    return system(cmd);
}

static void handle_sig(int sig) {
    if (sig==SIGINT||sig==SIGTERM) terminate=1;
}

static void *client_thread(void *arg) {
    int fd = *(int*)arg; free(arg);
    char buf[BUF_SZ];
    // 1) Autenticazione
    ssize_t n = recv(fd, buf, BUF_SZ-1, 0);
    if (n<=0) { close(fd); return NULL; }
    buf[n]=0;
    if (!auth_key || strcmp(buf, auth_key)!=0) {
        log_msg(LOG_ERR,"Auth failed from fd=%d",fd);
        send(fd, "AUTH_FAIL\n",10,0);
        close(fd);
        return NULL;
    }
    send(fd, "OK\n",3,0);
    // 2) Loop comandi
    while(!terminate) {
        n = recv(fd, buf, BUF_SZ-1, 0);
        if (n<=0) break;
        buf[n]=0;
        if (buf[n-1]=='\n') buf[n-1]=0;
        if (strcmp(buf,"exit")==0) break;
        FILE *p = popen(buf,"r");
        if (!p) {
            snprintf(buf,BUF_SZ,"ERROR: %s\n", strerror(errno));
            send(fd, buf, strlen(buf),0);
            continue;
        }
        while(fgets(buf,BUF_SZ,p)) send(fd, buf, strlen(buf),0);
        pclose(p);
    }
    close(fd);
    return NULL;
}

static void usage(const char *p) {
    fprintf(stderr,
      "Server %s\n"
      "Usage: %s [OPTIONS]\n"
      " -p, --port PORT     listen port (default 12345)\n"
      " -k, --key TOKEN     auth token (mandatory)\n"
      " -u, --update        apt-get update && upgrade -y\n"
      " -d, --dry-run       stampa comandi senza eseguirli\n"
      " -s, --syslog        abilit y syslog (default on)\n"
      " -V, --version       show version\n"
      " -h, --help          this help\n",
      VERSION, p);
    exit(1);
}

int main(int argc, char **argv) {
    int port = 12345, opt, yes=1;
    struct addrinfo hints = { .ai_family=AF_UNSPEC,
                              .ai_socktype=SOCK_STREAM,
                              .ai_flags=AI_PASSIVE }, *res;

    struct option longopts[] = {
        {"port",    required_argument, 0,'p'},
        {"key",     required_argument, 0,'k'},
        {"update",  no_argument,       0,'u'},
        {"dry-run", no_argument,       0,'d'},
        {"syslog",  no_argument,       0,'s'},
        {"version", no_argument,       0,'V'},
        {"help",    no_argument,       0,'h'},
        {0,0,0,0}
    };

    while ((opt=getopt_long(argc,argv,"p:k:udsVh",longopts,NULL))!=-1) {
        switch(opt) {
        case 'p': port       = atoi(optarg); break;
        case 'k': auth_key   = optarg;       break;
        case 'u': do_update  = 1;            break;
        case 'd': dry_run    = 1;            break;
        case 's': /* syslog già abilitato */ break;
        case 'V': printf("server %s\n",VERSION); exit(0);
        case 'h':
        default: usage(argv[0]);
        }
    }
    if (!auth_key) usage(argv[0]);

    openlog("remote-server", LOG_PID|LOG_CONS, LOG_DAEMON);
    signal(SIGINT, handle_sig);
    signal(SIGTERM,handle_sig);

    // update system
    if (do_update) {
        run_cmd("apt-get update -y");
        run_cmd("apt-get upgrade -y");
    }

    // setup socket
    char portstr[6]; snprintf(portstr,sizeof(portstr),"%d",port);
    if (getaddrinfo(NULL,portstr,&hints,&res)!=0) {
        perror("getaddrinfo"); exit(1);
    }
    sockfd = socket(res->ai_family,res->ai_socktype,0);
    setsockopt(sockfd,SOL_SOCKET,SO_REUSEADDR,&yes,sizeof(yes));
    bind(sockfd,res->ai_addr,res->ai_addrlen);
    listen(sockfd,BACKLOG);
    log_msg(LOG_INFO,"Listening on port %d %s",port,
            dry_run?"(dry-run)":"");

    while(!terminate) {
        struct sockaddr_storage ss; socklen_t sl=sizeof(ss);
        int *cfd=malloc(sizeof(int));
        *cfd = accept(sockfd,(void*)&ss,&sl);
        if (*cfd<0) { free(cfd); continue; }
        pthread_t tid;
        pthread_create(&tid,NULL,client_thread,cfd);
        pthread_detach(tid);
    }

    log_msg(LOG_INFO,"Shutting down"); 
    close(sockfd);
    closelog();
    return 0;
}
