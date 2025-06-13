// common.h
#ifndef COMMON_H
#define COMMON_H

#ifdef _WIN32
  #include <winsock2.h>
  #include <ws2tcpip.h>
  #pragma comment(lib,"ws2_32.lib")
  typedef SOCKET         socket_t;
  #define close_socket(s) closesocket(s)
  #define sleep_ms(ms)    Sleep(ms)
  static inline void init_net(void) {
    WSADATA w; WSAStartup(MAKEWORD(2,2), &w);
  }
  static inline void cleanup_net(void) {
    WSACleanup();
  }
  typedef HANDLE         thread_t;
  #define thread_create(h,f,a) \
    (h)=CreateThread(NULL,0,(LPTHREAD_START_ROUTINE)f,a,0,NULL)
#else
  #include <sys/socket.h>
  #include <netdb.h>
  #include <unistd.h>
  #include <arpa/inet.h>
  #include <pthread.h>
  typedef int           socket_t;
  #define close_socket(s) close(s)
  #define sleep_ms(ms)    usleep((ms)*1000)
  static inline void init_net(void) {}
  static inline void cleanup_net(void) {}
  typedef pthread_t     thread_t;
  #define thread_create(h,f,a) pthread_create(&(h),NULL,f,a)
#endif

#endif // COMMON_H
