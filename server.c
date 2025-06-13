// server.c
#include "common.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <stdarg.h>
#include <errno.h>

#define BACKLOG 10
#define BUF_SZ  4096
#define VERSION "1.0.0"

static int            dry_run   = 0;
static int            do_update = 0;
static char          *auth_key  = NULL;
static socket_t       sockfd;
static volatile int   terminate = 0;

static void logp(const char *fmt, ...) {
  va_list ap; va_start(ap, fmt);
  vprintf(fmt, ap); printf("\n");
  va_end(ap);
}

static int run_cmd(const char *cmd) {
  if (dry_run) {
    logp("[DRY] %s", cmd);
    return 0;
  }
  logp("[RUN] %s", cmd);
  return system(cmd);
}

static void handle_sig(int sig) {
  (void)sig; terminate = 1;
}

static void *client_thread(void *arg) {
  socket_t fd = (socket_t)(uintptr_t)arg;
  char buf[BUF_SZ];
  ssize_t n;

  /* Authentication */
  n = recv(fd, buf, BUF_SZ-1, 0);
  if (n <= 0) goto out;
  buf[n] = 0;
  if (strcmp(buf, auth_key) != 0) {
    send(fd, "AUTH_FAIL\n", 10, 0);
    goto out;
  }
  send(fd, "OK\n", 3, 0);

  while (!terminate) {
    n = recv(fd, buf, BUF_SZ-1, 0);
    if (n <= 0) break;
    buf[n] = 0;
    if (buf[n-1] == '\n') buf[n-1] = 0;
    if (strcmp(buf, "exit") == 0) break;

    FILE *p = popen(buf, "r");
    if (!p) {
      snprintf(buf, BUF_SZ, "ERROR: %s\n", strerror(errno));
      send(fd, buf, strlen(buf), 0);
      continue;
    }
    while (fgets(buf, BUF_SZ, p))
      send(fd, buf, strlen(buf), 0);
    pclose(p);
  }

out:
  close_socket(fd);
  return NULL;
}

int main(int argc, char **argv) {
  int port = 12345, yes = 1;
  thread_t tid;

  init_net();
  signal(SIGINT, handle_sig);
  signal(SIGTERM, handle_sig);

  /* parse options */
  for (int i = 1; i < argc; i++) {
    if (strcmp(argv[i], "-u") == 0)      do_update = 1;
    else if (strcmp(argv[i], "-d") == 0) dry_run   = 1;
    else if (strcmp(argv[i], "-k") == 0 && i+1<argc) auth_key = argv[++i];
    else if (strcmp(argv[i], "-p") == 0 && i+1<argc) port     = atoi(argv[++i]);
  }
  if (!auth_key) {
    fprintf(stderr, "ERROR: auth key required (-k TOKEN)\n");
    return 1;
  }

  if (do_update) {
    run_cmd("apt-get update -y");
    run_cmd("apt-get upgrade -y");
  }

  /* setup listener */
  struct addrinfo hints = {
    .ai_family   = AF_UNSPEC,
    .ai_socktype = SOCK_STREAM,
    .ai_flags    = AI_PASSIVE
  }, *res;
  char portstr[6];
  snprintf(portstr, sizeof(portstr), "%d", port);
  if (getaddrinfo(NULL, portstr, &hints, &res) != 0) {
    perror("getaddrinfo"); return 1;
  }
  sockfd = socket(res->ai_family, res->ai_socktype, 0);
  setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes));
  bind(sockfd, res->ai_addr, res->ai_addrlen);
  listen(sockfd, BACKLOG);
  freeaddrinfo(res);

  logp("Server v%s listening on port %d%s",
       VERSION, port, dry_run ? " (dry-run)" : "");

  while (!terminate) {
    struct sockaddr_storage ss;
    socklen_t sl = sizeof(ss);
    socket_t cfd = accept(sockfd, (struct sockaddr *)&ss, &sl);
    if (cfd < 0) continue;
    thread_create(tid, client_thread, (void *)(uintptr_t)cfd);
  }

  logp("Shutting down");
  close_socket(sockfd);
  cleanup_net();
  return 0;
}
