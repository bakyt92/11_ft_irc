// пример из книги Сетевое программирование от Биджа. Использование интернет сокетов.

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>

#define PORT "3490"
#define BACKLOG 10   // размер очереди ожидающих подключений

void sigchld_handler(int s) {
  while(waitpid(-1, NULL, WNOHANG) > 0);
  (void)s;
}

void *get_in_addr(struct sockaddr *sa) {                             // получить sockaddr, IPv4 или IPv6
  if (sa->sa_family == AF_INET)
    return &(((struct sockaddr_in*)sa)->sin_addr);
  return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int main(void) {
  int                     sockfd, new_fd, rv, yes=1;                 // слушать на sock_fd, новое подключение на new_fd
  struct addrinfo         hints, *servinfo, *p;
  struct sockaddr_storage their_addr;                                // адресная информация подключившегося
  socklen_t               sin_size;
  struct sigaction        sa;
  char                    s[INET6_ADDRSTRLEN];

  memset(&hints, 0, sizeof hints);
  hints.ai_family   = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags    = AI_PASSIVE;                                    // использовать мой IP
  if ((rv = getaddrinfo(NULL, PORT, &hints, &servinfo)) != 0)
    return (fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv)), 1);
  for(p = servinfo; p != NULL; p = p->ai_next) {                     // цикл по всем результатам и связывание с первым возможным
    if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
      perror("server: socket");
      continue;
    }
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
      perror("setsockopt");
      exit(1);
    }
    if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
      close(sockfd);
      perror("server: bind");
      continue;
    }
    break;
  }
  if (p == NULL)
    return (fprintf(stderr, "server: failed to bind\n"), 2);
  freeaddrinfo(servinfo);
  if (listen(sockfd, BACKLOG) == -1) {
    perror("listen");
    exit(1);
  }
  sa.sa_handler = sigchld_handler;                              // удалить мёртвые процессы
  sigemptyset(&sa.sa_mask);
  sa.sa_flags = SA_RESTART;
  if (sigaction(SIGCHLD, &sa, NULL) == -1) {                    // уборка зомби, которые появляются при завершении потомка после fork()
    perror("sigaction");
    exit(1);
  }
  printf("server: waiting for connections…\n");
  while(1) {                                                    // главный цикл accept()
  sin_size = sizeof their_addr;
  new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size);
  if (new_fd == -1) {
    perror("accept");
    continue;
  }
  inet_ntop(their_addr.ss_family,
  get_in_addr((struct sockaddr *)&their_addr), s, sizeof s);
  printf("server: got connection from %s\n", s);
  if (!fork()) {                                                // порождённый процесс
    close(sockfd);                                              // его слушать не нужно
    if (send(new_fd, "Hello, world!", 13, 0) == -1)
      perror("send");
    close(new_fd);
    exit(0);
    }
  }
  close(new_fd);                                               // родителю это не нужно
  return 0;
}
