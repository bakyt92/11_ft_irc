group: https://github.com/bakyt92/11_ft_irc/blob/master/docs/plan.md   
IRC = Internet Relay Chat  
* 1988 г
* для группового общения, общаться через личные сообщения и обмениваться файлами
* на основе IRC разработано множество мессенджеров, такие как: ICQ, Skype, Discord, Telegram, Slack, etc...
* * **most public IRC servers don't usually set a connection password**
 
## Сообщения (Internet Relay Chat Protocol)
* https://www.lissyara.su/doc/rfc/rfc1459/ 
* Серверы и клиенты создают сообщения на которые можно ответить, а можно и нет
* Если сообщение содержит правильные команды, то клиенту следует ответить как полагается, но это не означает, что всегда можно дождаться ответа
* IRC-сообщения = строка символов
  + заканчивается  парой символов CR-LF
  + длина строки не превышает 512 символов (в 512 входят CR-LF) => максимальная длина строки для команд и параметров - 510 символов
* Каждое IRC-сообщение может содержать до трех главных частей:
  + эти три части разделены одним (или более) символом ` `
  + 1) префикс (опционально)
     + обозначается одним символом `:` в первой позиции
     + между префиксом и двоеточием не должно быть пробелов
     + используется серверами для обозначения источника появления сообщения
     + если префикс утерян, то за источник сообщения берут соединение, с которого было получено сообщение
     + клиентам не следует пользоваться префиксами при отсылке сообщения (принимаются только правильные префиксы и только с зарегистрированных никнеймов, если идентификаторы префиксов не будет найдены в серверных базах данных, или если они зарегистрированы с различных линков, то сервер будет игнорировать сообщение)
  + 2) команда
    + правильная IRC-команда или трехзначное число, представленное в ASCII-тексте
  + 3) параметры команды
* Перенос строки невозможен
* `<reply> ::= <nick>['*'] '=' <hostname>` '*' обозначает, что клиент зарегистрирован как IRC-оператор
* Специальнае случаи:
  + `:Angel!localhost PRIVMSG Wiz`       a message from Angel to Wiz
  + `PRIVMSG jto@localhost :Hello`       a message to a user "jto" on server localhost
  + `PRIVMSG kalt%localhost`             a message to a user on the local server with username of "kalt", and connected from the localhost
  + `PRIVMSG Wiz!jto@localhost :Hello` a message to the user with nickname Wiz who is connected from the localhostand has the username "jto"
*  most public IRC servers don't usually set a connection password
  + PASS only required if a password is required to connect to a serve

## TCP
* гарантирует надёжность с точки зрения потока (UDP не гарантирует) 
* есть ненадёжность машин в сети
* не гарантирует, что каждый send() будет принят (recv()) соединением
* TCP какое-то время хранит данные в своём буфере отправки, но в конечном итоге сбросит данные по таймауту
* send() возвращает успешное выполнение, не зная, было ли сообщение успешного получено на стороне recv()
* в случае разъединения потеряем данные
* разработчик решает, как приложение реагирует на неожиданные разъединения
  + насколько сильно вы будете пытаться узнать, действительно ли получающее приложение получило каждый бит
  + подтверждающие сообщения
  + разметка сообщений идентификаторами
  + создание буфера
  + повторная отправка сообщений
  + таймауты, связанные с каждым сообщением
  + хранить пакеты данных на жёстком диске, тогда если возникнет сбой, то позже попытаться отправить эти данные
  + разъединение во время длительных операций; когда две машины восстановят связь, можно будет передать результаты операции
  + определять, насколько важно подтверждать доставку в каждом конкретном случае
  + прикладывать большие усилия к тому, чтобы подтвердить завершение длительной операции, но смириться с утерей данных, сообщающих о степени выполнения операции
  + многие приложения завершают работу, если происходит отключение в неожиданный момент
  + отдельный цикл переподключения, который будет какое-то время спать, а затем пытаться переподключиться и если это удастся, продолжить обычную работу
  + обрабатывать потенциальные разъединения при любом вызове recv()
  + обрабатывать потенциальные ошибки разъединения при каждом вызове
  + отключить оповещения при recv(), чтобы обрабатывать ошибку подключения линейно, не регистрировать для этого обработчик сигналов

## `int poll(struct pollfd *fds, nfds_t nfds, int délai)`
* ожидает некоторое событие над файловым дескриптором
* ждёт, пока один дескриптор из набора файловых дескрипторов не станет готов выполнить операцию ввода-вывода
* the OS marks each of fd-s with the kind of event that occurred
* если ни одно из запрошенных событий с файловыми дескрипторами не произошло или не возникло ошибок, то блокируется до их появления
* fds = l'ensemble des descripteurs de fichier à surveiller
* nfds = количество элементов в fds
  + если nfds 0, то
    - поле events игнорируется
    - полю revents возвращает ноль
    - это простой способ игнорирования файлового дескриптора в одиночном вызове poll()
    - это нельзя использовать для игнорирования файлового дескриптора 0
* better than select, because you can keep re-using the same data structure
* POSIX: if the value of fd is less than 0, events shall be ignored, and revents shall be set to 0 in that entry on return from poll()
* returns:
  + le nombre de structures ayant un champ revents non nul = le nombre de structures pour lesquels un événement attendu
  + NULL: un dépassement du délai d'attente et qu'aucun descripteur de fichier n'était prêt
  + -1: s'ils échouent, errno contient le code d'erreur  
```
struct pollfd {
    int   fd;         
    short events;     /* Événements attendus    */
    short revents;    /* Événements détectés    */
};
```
## epoll()
* improves upon the mechanism
* http://scotdoyle.com/python-epoll-howto.html

## kqueue()
...

## select() 
* three bitmasks to mark which fd-s you want to watch for reading, writing, errors
* the OS marks which ones have had some kind of activity
* clunky and inefficient

## `listen(int s, int backlog)`
1. listen слушает подключающихся
2. пришел запрос на подключение
3. программа пошла дальше, чтобы сделать accept - но пока программа не дошла до accept, могут приходить запросы на подключение от других клиентов (bqcklog определяет, сколько их может накопиться в очереди на соединение)
4. программа выбирает accept
5. создаётся новый сокет, который связывает программу с определенным клиентом, дальше программа работает с этим сокетом
5. cервер с клиентом общаются по сокету (появившемуся после accept), а listen в это время делается на другом, пассивном сокете
* s = сокет для прослушивания
* backlog = целое положительное число, определяющее, как много запросов связи может быть принято на сокет одновременно
  + в большинстве систем это значение должно быть не больше пяти
  + имеет отношение только к числу запросов на соединение, которые приходят одновременно
  + число установленных соединений может превышать это число
  + не имеет отношения к числу соединений, которое может поддерживаться сервером
* sets up the listening socket's backlog and opens the bound port, so clients can start connecting to the socket
  + that opening is a very quick operation, there is no need to worry about it blocking
  + a 3-way handshake is not performed by listen()
  + it is performed by the kernel when a client tries to connect to the opened port and gets placed into the listening socket's backlog
  + each new client connection performs its own 3-way handshake
* listen() does not bock
* once a client connection is fully handshaked, that connection is made available for accept() to extract it from the backlog
* you call listen() only 1 time, to open the listening port, that is all it does

## `accept()`
* accept() blocks
* blocks (or, if you use a non-blocking listening socket, accept() succeeds) only when a new client connection is available for subsequent communication
* you have to call accept() for each client that you want to communicate with

## send()
* send (2) is a POSIX stqndqnt for system calls
* is generally associated with low level system calls
* transmit a message to another socket
* may be used only when the socket is in a connected state (so that the intended recipient is known)
* the only difference between send() and write(2) is the presence of flags
* `ssize_t send(int sockfd, const void buf[.len], size_t len, int flags)`
* `ssize_t sendto(int sockfd, const void buf[.len], size_t len, int flags, const struct sockaddr *dest_addr, socklen_t addrlen)`
* `ssize_t sendmsg(int sockfd, const struct msghdr *msg, int flags)`
* флаг MSG_NOSIGNAL
  + = requests not to send the SIGPIPE signal if an attempt to send is made on a stream-oriented socket that is no longer connected
  + don't generate a SIGPIPE signal if the peer has closed the connection
* if there is not enough available buffer space to hold the socket data to be transmitted
  + if the socket is in blocking mode, **send() blocks** the caller until additional buffer space becomes available
  + if the socket is in nonblocking mode, send() returns -1 and sets the error code to EWOULDBLOCK
* `send(3)` `ssize_t send(int socket, const void *bufr, size_t leng, int flags)`
  + associated with high-level functions

## recv recvfrom recvmsg
* if a message is too long to fit in the supplied buffer, excess bytes may be discarded depending on the type of socket the message is received from
* if no messages are available at the socket, the receive calls wait for a message to arrive, unless the socket is nonblocking
* the receive calls normally return any data available, up to the requested amount, rather than waiting for receipt of the full amount requested
* an application can use select, poll, epoll to determine when more data arrives on a socket
* flag MSG_DONTWAIT Enables nonblocking operation
  + if the operation would block, the call fails with the error EAGAIN or EWOULDBLOCK
  + similar behavior to setting the O_NONBLOCK flag via the fcntl
  + MSG_DONTWAIT is a per-call option
  + O_NONBLOCK is a setting on the open file description
* every function returns immediately, i.e. all the functions in such programs are nonblocking
* Instead, they use the "state machine" technique
* ...
* How to query the available data on a socket:
  + Non-bloking sockets
  + select()/poll()
* the call to poll has no effect on the call to recv, whether or not recv blocks depends on:
  + what data is available at the time you call recv
  + whether the socket is in blocking or non-blocking mode

### `ssize_t recv(int sockfd, void buf[.len], size_t len, int flags)`
* ждёт, пока данные не появятся
* вернёт -1, если сокет находится в неблокирующем режиме и данные ещё не пришли
* вернёт -1, если вызов будет прерван сигналом с установкой errno
* вернёт 0, если соединение закрыто второй стороной
* если вы считали 100 байт, а потом попытаетесь читать еще раз, то программа заблокируется на `recv` и будет стоять, пока не получит хотя бы байт, или сервер со своей стороны не закроет соединение (recv вернёт 0)
  + 3 варианта управлять этой ситуацией:
    - сделать сокет асинхронным, и тогда вы получите отрицательное значение, а в errono будет что-то типа EAGAIN (EWOULDBLOCK) - так система вам намекнет, что данных для вас у нее нет
    - сделать не весь сокет неблокирующим, а только данную операцию, подставив флаг MSG_NONBLOCK
    - перед чтением проверять, что в сокет что-то пришло. Это всякие poll, select и их вариации
*  The only difference between recv() and read(2) is the presence of flags

### `ssize_t recvfrom(int sockfd, void buf[restrict .len], size_t len, int flags, struct sockaddr *_Nullable restrict src_addr, socklen_t *_Nullable restrict addrlen)`
* `recvfrom(sockfd, buf, len, flags, NULL, NULL)` = `recv(sockfd, buf, len, flags)`
           
### `ssize_t recvmsg(int sockfd, struct msghdr *msg, int flags)`

## Requirements
* multiple clients at the same time
* never hang
* forking is not allowed
* I/O operations must be non-blocking
* only 1 poll() can be used for handling all I/O operations (read, write, listen, ...)
* non-blocking file descriptors => we may use read/recv or write/send functions with no poll()
  + ther server is not blocking
  + it consumes more system resources
* forbdden:  read/recv,r write/send without poll() (in any file descriptor)
* communication between client and server has to be done via TCP/IP (v4 or v6)
* Verify absolutely every possible error and issue (receiving partial data, low bandwidth, ...)
* **In order to process a command, you have to first aggregate the received packets in order to rebuild it**
      
## You only have to implement the following features
  + INVITE: Invite a client to a channel (only by channel operators)
  + TOPIC: to change or view the channel topic (only by channel operators)
    - if <topic> is given, it sets the channel topic to <topic>
  + MODE: Change the channel’s mode (only by channel operators):
    - i: Invite-only channel
    - t: the restrictions of the TOPIC command to channel operators
    - k: the channel key (password)
    - o: Give/take channel operator privilege
    - l: Set/remove the user limit to channel

