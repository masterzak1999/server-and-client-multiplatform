// common.h
#ifndef COMMON_H
#define COMMON_H

#ifdef _WIN32
  #include <winsock2.h>
  #include <ws2tcpip.h>
  #pragma comment(lib,"ws2_32.lib")
  typedef SOCKET      socket_t;
  #define close_socket(s) closesocket(s)
  #define sleep_ms(ms)    Sleep(ms)
  static inline void init_net(void) { WSADATA w; WSAStartup(MAKEWORD(2,2), &w); }
  static inline void cleanup_net(void) { WSACleanup(); }
  typedef HANDLE      thread_t;
  #define thread_create(h,f,a) \
    (h)=CreateThread(NULL,0,(LPTHREAD_START_ROUTINE)f,a,0,NULL)
#else
  #include <sys/socket.h>
  #include <netdb.h>
  #include <unistd.h>
  #include <arpa/inet.h>
  #include <pthread.h>
  typedef int         socket_t;
  #define close_socket(s) close(s)
  #define sleep_ms(ms)    usleep((ms)*1000)
  static inline void init_net(void) {}
  static inline void cleanup_net(void) {}
  typedef pthread_t   thread_t;
  #define thread_create(h,f,a) pthread_create(&(h),NULL,f,a)
#endif

/* XOR-encrypt/decrypt buffer in place using key */
static inline void xor_buf(unsigned char *buf, size_t n,
                           const char *key, size_t keylen) {
  for (size_t i = 0; i < n; i++)
    buf[i] ^= key[i % keylen];
}

/* encrypted send/recv */
static inline ssize_t send_enc(socket_t s, const void *v, size_t n,
                               const char *key, size_t keylen) {
  unsigned char *tmp = malloc(n);
  memcpy(tmp, v, n);
  xor_buf(tmp, n, key, keylen);
  ssize_t r = send(s, tmp, n, 0);
  free(tmp);
  return r;
}
static inline ssize_t recv_enc(socket_t s, void *v, size_t n,
                               const char *key, size_t keylen) {
  ssize_t r = recv(s, v, n, 0);
  if (r > 0) xor_buf((unsigned char*)v, r, key, keylen);
  return r;
}

#endif // COMMON_H
