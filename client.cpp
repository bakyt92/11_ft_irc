#define PORT "3490"                                            // порт для подключения клиентов
#define MAXDATASIZE 100                                        // максимальная длина принимаемых за раз данных
void *get_in_addr(struct sockaddr *sa) {                       // получить sockaddr, IPv4 или IPv6
  if (sa->sa_family == AF_INET) 
    return &(((struct sockaddr_in*)sa)->sin_addr);
  return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

int main(int argc, char *argv[]) {
  int             sockfd, numbytes, rv;
  char            buf[MAXDATASIZE];
  struct addrinfo hints, *servinfo, *p;
  char            s[INET6_ADDRSTRLEN];

  if (argc != 2) {
    fprintf(stderr,"usage: client hostname\n");
    exit(1);
    }
  memset(&hints, 0, sizeof hints);
  hints.ai_family   = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  if ((rv = getaddrinfo(argv[1], PORT, &hints, &servinfo)) != 0) 
    return (fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv)), 1);
  for(p = servinfo; p != NULL; p = p->ai_next) {                        // цикл по всем результатам и связывание с первым возможным
    if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
      perror("client: socket");
      continue;
    }
    if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
      close(sockfd);
      perror("client: connect");
      continue;
    }
    break;
  }
  if (p == NULL)
    return (fprintf(stderr, "client: failed to connect\n"), 2);
  inet_ntop(p->ai_family, get_in_addr((struct sockaddr *)p->ai_addr), s, sizeof s);
  printf("client: connecting to %s\n", s);
  freeaddrinfo(servinfo);
  if ((numbytes = recv(sockfd, buf, MAXDATASIZE-1, 0)) == -1) {
    perror("recv");
    exit(1);
  }
  buf[numbytes] = ’\0’;
  printf("client: received ’%s’\n",buf);
  close(sockfd);
  return 0;
}
