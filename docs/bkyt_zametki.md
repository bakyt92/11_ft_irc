Заметки по книге 

Два типа сокетов

**1. Stream sockets**
Потоковые сокеты это надёжные подключаемые двунаправленные потоки связи. Если вы отправите в сокет два послания в порядке “1, 2”, то на другой стороне они также появятся в порядке “1, 2”. Они также будут свободными от ошибок (error-free) - telnet приложения, HTTP протокол.

_Как потоковые сокеты достигают столь высокого качества передачи данных? Они
используют протокол, называемый “The Transmission Control Protocol”, иначе известный
как “TCP” (см. RFC 793 с полной информацией по TCP). TCP обеспечивает появление 7
ваших данных последовательно и без ошибок. Может быть ранее вы встречали “TCP” как
первую часть “TCP/IP”, где “IP” означает “Internet Protocol” (см. RFC 791 ). IP изначально 8
работает с Интернет маршрутизацией и обычно не отвечает за целостность данных._

**2. Datagram sockets** - Дейтаграммные сокеты иногда называют “неподключаемыми” (без установки логического соединения), хотя если очень хочется их можно подключить connect()-ом. (См. ниже). Они в основном используют UDP (User Datagram Protocol) - tftp (ftp protocol) / dhcpcd (DHCP client).

**Инкапсуляция** производится на разных уровнях 
![image](https://github.com/bakyt92/11_ft_irc/assets/40731866/4e6e973f-381f-4b17-8ff6-d91cee75a4cd)

- в первую очередь приложение заполняет специальную структуру данных, в которой указывает информацию о получателе (сетевой протокол, IP-адрес, порт UDP);
- передаёт сообщение, его длину и структуру с информацией о получателе обработчику протокола UDP (транспортный уровень);
- обработчик UDP формирует датаграмму, в которой в качестве данных выступает сообщение, а в заголовках находится UDP-порт получателя (а также другие данные);
- обработчик UDP передаёт сформированную датаграмму обработчику IP (сетевой уровень);
- обработчик IP рассматривает переданную UDP датаграмму как данные и предваряет их своим заголовком (в котором, в частности, находится IP-адрес получателя, взятый из той же структуры данных приложения, и номер верхнего протокола);
полученный пакет обработчик IP передаёт на канальный уровень, который опять-таки рассматривает данный пакет как «сырые» данные;
- обработчик канального уровня, аналогично предыдущим обработчикам, добавляет в начало свой заголовок (в котором так же указывается номер протокола верхнего уровня, в нашем случае это 0x0800(IP)) и, в большинстве случаев, добавляет конечную контрольную сумму, тем самым формируя кадр;
- далее полученный кадр передаётся на физический уровень, который осуществляет преобразование битов в электрические или оптические сигналы и посылает их в среду передачи.

Порядок байтов может различаться между сетью и хостом. 
Для преобразования порядка байтов могут использоваться следующие команды:
- htons() host to network short
- htonl() host to network long
- ntohs() network to host short
- ntohl() network to host long

Дескриптор сокета носит тип int. 

Структура для подготовки адресных стрктур сокета для дальнейшего использования 
```
struct addrinfo 
{
 int ai_flags; // AI_PASSIVE, AI_CANONNAME, т.д.
 int ai_family; // AF_INET, AF_INET6, AF_UNSPEC
 int ai_socktype; // SOCK_STREAM, SOCK_DGRAM
 int ai_protocol; // используйте 0 для"any"
 size_t ai_addrlen; // размер ai_addr в байтах
 struct sockaddr *ai_addr; // struct sockaddr_in или _in6
 char *ai_canonname; // полное каноническое имя хоста
 struct addrinfo *ai_next; // связанный список, следующий
 };
```

Необходимо загрузить эту структуру и вызвать команду **getaddrinfo**(). Она возвращает указатель на новый связанный список этих структур, содержащих всё, что вам надо

Вы можете приказать использовать IPv4 или IPv6 в поле ai_family или оставить
AF_UNSPEC чтобы использовать любой. Это круто, поскольку ваш код сможет быть
независимым от версии IP.
Заметьте, что это связанный список: ai_next указывает на следующий элемент -
результатов может быть несколько, есть из чего выбирать. Я использую первый рабочий,
а у вас могут быть другие нужды. Я же всего не знаю, чувак!
Как видите поле ai_addr в структуре addrinfo это указатель на структуру
sockaddr. Вот мы и полезли во внутренности структур IP адреса.
Обычно вам нет нужды заполнять эти структуры, гораздо чаще достаточно вызвать
getaddrinfo() и всё нужное там. Однако, вы будете всматриваться во внутренности
этих структур, чтобы получить их значения, так что я их здесь представляю. 

```
struct sockaddr {
 unsigned short sa_family; // семейство адресов, AF_xxx
 char sa_data[14]; // 14 байт адреса протокола
}; 
```
sa_family может быть разнообразной, но будет либо AF_INET (IPv4) или AF_INET6
для всего, что мы делаем в этом документе.
sa_data содержит адрес назначения и номер порта для сокета. Это весьма громоздко,
но ведь вы не хотите утомительно упаковывать адрес в sa_data вручную.
Для работы со структурой sockaddr программисты создают параллельную структуру
struct sockaddr_in (“in” это “Internet”) для использования с IPv4.

И что важно, указатель на структуру sockaddr_in может быть приведен к указателю
на структуру sockaddr и наоборот. Так что даже хоть connect() и требует struct
sockaddr* вы всё равно можете пользоваться структурой sockaddr_in и привести её в
последний момент!
 // (Только для IPv4 — для IPv6 смотри struct sockaddr_in6)
 ```
struct sockaddr_in {
 short int sin_family; // Семейство адресов, AF_INET
 unsigned short int sin_port; // Номер порта
 struct in_addr sin_addr; // Интернет адрес
 unsigned char sin_zero[8]; // Размер как у struct sockaddr
};
```
```
// (Только для IPv6 —- для IPv4 смотри struct sockaddr_in и struct in_addr)
struct sockaddr_in6 {
 u_int16_t sin6_family; // семейство адресов, AF_INET6
 u_int16_t sin6_port; // номер порта, Порядок Байтов Сети
 u_int32_t sin6_flowinfo; // потоковая информация IPv6
 struct in6_addr sin6_addr; // адрес IPv6
 u_int32_t sin6_scope_id; // Scope ID
};
struct in6_addr {
 unsigned char s6_addr[16]; // адрес IPv6
}
```

```
struct sockaddr_storage{
 sa_family_t ss_family; // семейство адресов
 // это выравнивание длины, зависит от реализации, проигнорируйте
 char __ss_pad1[SS_PAD1SIZE];
 int64_t __ss_align;
 char __ss_pad2[SS_PAD2SIZE];
 };
```

Функция **inet_pton()** преобразует IP адрес в записи “цифрыточки” в структуру struct in_addr либо struct in6_addr в зависимости от указанных AF_INET или AF_INET6. (“pton” означает “presentation to network” или можете
называть “printable to network”, если так проще запомнить.

Для ipv6 - используется функция **inet_ntop()**(“ntop” означает “network to presentation” или можете называть “network to printable”

### Детализация функции **getaddrinfo()**
```
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
 int getaddrinfo(const char *node, // например, "www.example.com" или IP
 const char *service, // например, "http" или номер порта
 const struct addrinfo *hints,
 struct addrinfo **res);
```
gai_strerror() - позволяет распечатать ошибку. 

### Пример запуска процесса подключения клиента к серверу:

Вот пример вызова если вы клиент, который хочет подсоединиться к определённому
серверу, скажем, “www.example.net” порт 3490. Опять же это не настоящее подключение,
а заполнение структур, которые мы используем позднее:
```
int status;
struct addrinfo hints;
 struct addrinfo *servinfo; // укажет на результат
 memset(&hints, 0, sizeof hints); // очистка структуры
 hints.ai_family = AF_UNSPEC; // IPv4 либо IPv6
 hints.ai_socktype = SOCK_STREAM; // потоковый сокет TCP
 // готовьтесь к соединению
 status = getaddrinfo("www.example.net", "3490", &hints, &servinfo);
 // servinfo теперь указывает на связанный список из 1 или более struct addrinfo
 // и т.д.
```

### Пример функции socket

```
#include <sys/types.h>
 #include <sys/socket.h>
 int socket(int domain, int type, int protocol); 
```

Для запуска сокета необходимо взять данные из результатов вызова getaddrinfo() и передать их socket(), прямо как здесь:
```
int s;
 struct addrinfo hints, *res;
 // поиск
 // [полагаем, что структура “hints" уже заполнена]
 getaddrinfo("www.example.com", "http", &hints, &res);
 // [нужно проверить выход getaddrinfo() на ошибки и просмотреть
 // связанный список "res" на действительный элемент, не полагаясь
 // на то, что это первый (как во многих других примерах.)]
 // [Примеры смотри в разделе Клиент-Сервер.]
s = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
```
socket() просто возвращает вам дескриптор сокета, который вы можете позже использовать в системных вызовах или -1 в случае ошибки. Глобальная переменная
errno содержит код ошибки (см. подробности в *man* странице errno и кратких заметках по использованию errno в многопоточных программах.)


Подключение сокета через команду *connect* - подключаем всокет к example.com 
```
struct addrinfo hints, *res; int sockfd;
// сначала заполнить адресные структуры с помощью getaddrinfo():
memset(&hints, 0, sizeof hints); hints.ai_family = AF_UNSPEC; hints.ai_socktype = SOCK_STREAM;
getaddrinfo("www.example.com", "3490", &hints, &res);
// создать сокет:
sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol); // подключить!
connect(sockfd, res->ai_addr, res->ai_addrlen);
```

Вызов listen() очаровательно прост, но требует некоторого разъяснения: 
```
int listen(int sockfd, int backlog);
```
- sockfd это обычный файловый дескриптор сокета из системного вызова socket().
- backlog это число разрешённых входных подключений во входной очереди. 

### accept ()

```
#include <string.h> 
#include <sys/types.h> 
#include <sys/socket.h> 
#include <netinet/in.h>
#define MYPORT "3490"
#define BACKLOG 10 
int main(void)
{
// номер моего порта для подключения пользователей // размер очереди ожидающих подключений
struct sockaddr_storage their_addr; socklen_t addr_size;
struct addrinfo hints, *res; int sockfd, new_fd;
// !! не забудьте проверить ошибки для этих вызовов !!
// сначала заполнить адресные структуры с помощью getaddrinfo():
memset(&hints, 0, sizeof hints);
hints.ai_family = AF_UNSPEC; // использовать либо IPv4 либо IPv6 hints.ai_socktype = SOCK_STREAM;
hints.ai_flags = AI_PASSIVE; // заполнить мой IP для меня
getaddrinfo(NULL, MYPORT, &hints, &res); // создать сокет, связать и слушать:
sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol); bind(sockfd, res->ai_addr, res->ai_addrlen);
listen(sockfd, BACKLOG);
// принять входящие подключения:
addr_size = sizeof their_addr;
new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &addr_size);
// связываемся по дескриптору сокета new_fd!
}
```

### send / recv 

Эти команды обеспечивают связь по потоковым и подключенным дейтаграммным сокетам 

*Вызов send():*
```
int send(int sockfd, const void *msg, int len, int flags);
```
- sockfd это дескриптор сокета, куда вы хотите отправить данные (возможно он был возвращён socket() или получен accept().)
- msg это указатель на посылаемые данные.
- len это длина этих данных в байтах.
- flags просто установите в 0. (См. man страницу вызова send(), там есть информация относительно flags.)


*Вызов recv()* во многом подобен:
```
int recv(int sockfd, void *buf, int len, int flags);
```
- sockfd это дескриптор сокета для чтения,
- buf это буфер куда читать,
- len это максимальная длина буфера,
- flags can опять может быть установлен в 0 (см. man страницу recv().)
- recv() возвращает действительное количество записанных в буфер байтов или -1 при ошибке ( и соответственно установив errno).
- recv() может возвращать 0. Это означает, что удалённая сторона закрыла для вас подключение! Это способ сказать вам об этом.

sendto() / recvfrom() - для неподключаемых сокетов (DGRAM)

### close () / shutdown ()

Закрытие файлового дескриптора происходит через команду close()
shutdown - чуть больше управления над закрытием сокета, разорвать связь между сервером и клиентом, как в одном направлении, так и в нескольких. 

### getpeername()
Функция скажет кто находится на другом конце подключенного потокового сокета. 

### gethostname()
Возвращает имя компьютера на котором запущена программа 
gethostbyname() для определения IP адреса вашей локальной машин

### Есть большой пример кода на странице 31-32 

### Блокировки
Если не хотим блокировки сокета, то необходимо вызвать fcntl()

### select()
Мультиплексирование синхронного ввода и вывода. 
select() даёт вам возможность следить за несколькими сокетами одновременно. Она скажет вам какие готовы для чтения, какие для записи, а какие возбудили исключение,
если вы действительно хотите это знать.

### poll()

       poll() performs a similar task to select(2): it waits for one of
       a set of file descriptors to become ready to perform I/O.  The
       Linux-specific epoll(7) API performs a similar task, but offers
       features beyond those found in poll().

       The set of file descriptors to be monitored is specified in the
       fds argument, which is an array of structures of the following
       form:
```
           struct pollfd {
               int   fd;         /* file descriptor */
               short events;     /* requested events */
               short revents;    /* returned events */
           };
```
       The caller should specify the number of items in the fds array in
       nfds.