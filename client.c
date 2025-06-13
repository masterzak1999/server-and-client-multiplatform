// client.c
#include "common.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BUF_SZ 4096

static void usage(const char *prog) {
  fprintf(stderr,
    "Usage: %s -h HOST -p PORT -k TOKEN [-d]\n", prog);
  exit(1);
}

int main(int argc, char **argv) {
  char *host = NULL, *port = NULL, *key = NULL;
  int  dry_run = 0;

  for (int i = 1; i < argc; i++) {
    if (!strcmp(argv[i], "-h") && i+1<argc) host = argv[++i];
    else if (!strcmp(argv[i], "-p") && i+1<argc) port = argv[++i];
    else if (!strcmp(argv[i], "-k") && i+1<argc) key  = argv[++i];
    else if (!strcmp(argv[i], "-d"))             dry_run = 1;
  }
  if (!host || !port || !key) usage(argv[0]);

  init_net();
  socket_t sock;
  if (!dry_run) {
    struct addrinfo hints = {
      .ai_family   = AF_UNSPEC,
      .ai_socktype = SOCK_STREAM
    }, *res;
    getaddrinfo(host, port, &hints, &res);
    sock = socket(res->ai_family, res->ai_socktype, 0);
    connect(sock, res->ai_addr, res->ai_addrlen);
    freeaddrinfo(res);

    send_enc(sock, key, strlen(key), key, strlen(key));
    send_enc(sock, "\n", 1, key, strlen(key));

    char ack[BUF_SZ];
    recv_enc(sock, ack, sizeof(ack), key, strlen(key));
    if (strncmp(ack, "OK", 2) != 0) {
      fprintf(stderr, "Auth failed\n");
      return 1;
    }
    printf("Connected to %s:%s\n", host, port);
  }

  char buf[BUF_SZ];
  while (fgets(buf, BUF_SZ, stdin)) {
    buf[strcspn(buf, "\n")] = '\0';
    if (strcmp(buf, "exit") == 0) break;

    if (dry_run) {
      printf("[DRY] %s\n", buf);
      continue;
    }
    if (!strncasecmp(buf, "GET ", 4) || !strncasecmp(buf, "PUT ", 4)) {
      send_enc(sock, buf, strlen(buf), key, strlen(key));
      send_enc(sock, "\n", 1, key, strlen(key));
      /* handle in server */
      continue;
    }

    send_enc(sock, buf, strlen(buf), key, strlen(key));
    send_enc(sock, "\n", 1, key, strlen(key));

    ssize_t n;
    while ((n = recv_enc(sock, buf, BUF_SZ, key, strlen(key))) > 0)
      printf("%.*s", (int)n, buf);
  }

  if (!dry_run) close_socket(sock);
  cleanup_net();
  return 0;
}
