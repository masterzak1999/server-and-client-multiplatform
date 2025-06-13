// client.c
#include "common.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BUF_SZ 4096
#define VERSION "1.0.0"

static void usage(const char *prog) {
  fprintf(stderr,
    "Client %s\n"
    "Usage: %s -h HOST -p PORT -k TOKEN [-d]\n"
    "  -h HOST   server address\n"
    "  -p PORT   server port\n"
    "  -k TOKEN  auth token\n"
    "  -d        dry-run\n",
    VERSION, prog);
  exit(1);
}

int main(int argc, char **argv) {
  char *host = NULL, *port = NULL, *key = NULL;
  int dry_run = 0;

  for (int i = 1; i < argc; i++) {
    if (strcmp(argv[i], "-h") == 0 && i+1<argc) host = argv[++i];
    else if (strcmp(argv[i], "-p") == 0 && i+1<argc) port = argv[++i];
    else if (strcmp(argv[i], "-k") == 0 && i+1<argc) key  = argv[++i];
    else if (strcmp(argv[i], "-d") == 0)             dry_run = 1;
  }
  if (!host || !port || !key) usage(argv[0]);

  init_net();
  if (!dry_run) {
    struct addrinfo hints = {
      .ai_family   = AF_UNSPEC,
      .ai_socktype = SOCK_STREAM
    }, *res;
    if (getaddrinfo(host, port, &hints, &res) != 0) {
      perror("getaddrinfo"); return 1;
    }
    socket_t sock = socket(res->ai_family, res->ai_socktype, 0);
    if (connect(sock, res->ai_addr, res->ai_addrlen) != 0) {
      perror("connect"); return 1;
    }
    freeaddrinfo(res);

    /* send token */
    send(sock, key, strlen(key), 0);
    send(sock, "\n", 1, 0);
    char ack[BUF_SZ];
    if (recv(sock, ack, sizeof(ack)-1, 0) <= 0 ||
        strncmp(ack, "OK", 2) != 0) {
      fprintf(stderr, "Auth failed\n");
      close_socket(sock);
      return 1;
    }
    printf("Connected to %s:%s\n", host, port);

    /* interactive shell */
    while (fgets(ack, sizeof(ack), stdin)) {
      if (ack[strlen(ack)-1] == '\n')
        ack[strlen(ack)-1] = '\0';
      if (strcmp(ack, "exit") == 0) break;
      send(sock, ack, strlen(ack), 0);
      send(sock, "\n", 1, 0);
      ssize_t n;
      while ((n = recv(sock, ack, sizeof(ack)-1, MSG_DONTWAIT)) > 0) {
        ack[n] = '\0';
        printf("%s", ack);
      }
    }
    close_socket(sock);
  }
  else {
    printf("[DRY] Would connect %s:%s with key '%s'\n",
           host, port, key);
  }
  cleanup_net();
  return 0;
}
