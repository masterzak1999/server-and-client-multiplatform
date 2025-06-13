// client.c v3
// Remote-Cmd Client con auth, dry-run, help, version

#define _GNU_SOURCE
#include <arpa/inet.h>
#include <errno.h>
#include <getopt.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define BUF_SZ 4096
#define VERSION "v3.0.0"

static void usage(const char *p) {
    fprintf(stderr,
      "Client %s\n"
      "Usage: %s [OPTIONS]\n"
      " -h, --host HOST     server hostname/IP\n"
      " -p, --port PORT     server port\n"
      " -k, --key TOKEN     auth token\n"
      " -d, --dry-run       stampa comandi senza inviarli\n"
      " -V, --version       show version\n"
      " -? ,--help          this help\n",
      VERSION, p);
    exit(1);
}

int main(int argc, char **argv) {
    char *host=NULL, *port=NULL, *key=NULL;
    int dry_run=0, opt;

    struct option longopts[] = {
        {"host",    required_argument,0,'h'},
        {"port",    required_argument,0,'p'},
        {"key",     required_argument,0,'k'},
        {"dry-run", no_argument,      0,'d'},
        {"version", no_argument,      0,'V'},
        {"help",    no_argument,      0,'?'},
        {0,0,0,0}
    };

    while((opt=getopt_long(argc,argv,"h:p:k:dV?",longopts,NULL))!=-1){
      switch(opt){
        case 'h': host  = optarg; break;
        case 'p': port  = optarg; break;
        case 'k': key   = optarg; break;
        case 'd': dry_run=1;      break;
        case 'V': printf("client %s\n",VERSION); exit(0);
        default:  usage(argv[0]);
      }
    }
    if (!host||!port||!key) usage(argv[0]);

    if (dry_run) {
        printf("[DRY] Would connect to %s:%s with key '%s'\n",
               host,port,key);
    }

    struct addrinfo hints = { .ai_family=AF_UNSPEC,
                              .ai_socktype=SOCK_STREAM}, *res;
    if (!dry_run && getaddrinfo(host,port,&hints,&res)!=0) {
        perror("getaddrinfo"); return 1;
    }
    int sock = -1;
    if (!dry_run) {
        sock = socket(res->ai_family,res->ai_socktype,0);
        if (sock<0||connect(sock,res->ai_addr,res->ai_addrlen)<0) {
            perror("connect"); return 1;
        }
    }
    freeaddrinfo(res);

    // invia key
    if (!dry_run) {
        send(sock,key,strlen(key),0);
        send(sock,"\n",1,0);
        char ack[32];
        ssize_t n = recv(sock,ack,sizeof(ack)-1,0);
        if (n<=0||strncmp(ack,"OK",2)!=0) {
            fprintf(stderr,"Auth failed: %s\n", ack);
            close(sock);
            return 1;
        }
    }

    printf("Connected to %s:%s %s\n", host, port,
           dry_run?"(dry-run)":"");
    char line[BUF_SZ];
    while(fgets(line,sizeof(line),stdin)) {
        if (line[strlen(line)-1]=='\n') line[strlen(line)-1]=0;
        if (strcmp(line,"exit")==0) break;
        if (dry_run) {
            printf("[DRY] Would send: %s\n", line);
            continue;
        }
        send(sock,line,strlen(line),0);
        send(sock,"\n",1,0);
        // recv risposta
        ssize_t n;
        while((n=recv(sock,line,BUF_SZ-1,MSG_DONTWAIT))>0) {
            line[n]=0; fputs(line,stdout);
        }
    }

    if (!dry_run) close(sock);
    return 0;
}
