# Сетевое программирование от Биджа. Использование интернет	сокетов. (сокращённое изложение)
https://github.com/bakyt92/11_ft_irc/blob/master/docs/bgnet_A4_rus.pdf  

## socket
* как Unix-программа выполняет любую операцию ввода/вывода? чтением или записью в дескриптор файла
* дескриптор файла = целое число, связанное с открытым файлом
* файл = сетевое подключение, FIFO, конвейер, терминал, реальный файл на диске и всё, что угодно
* `send(), recv()` ≈ `read(), write()`, но больше возможностей управления передачей данных

### stream socket
* internet-socket
* подключаемый двунаправленный поток связи
* используется в telnet приложениях
* для загрузки страниц web браузерами по HTTP протоколу
  + если подключиться к web сайту по telnet-у и, напечатать “GET / HTTP/1.0”, получишь html
* The Transmission Control Protocol (TCP) обеспечивает появление данных последовательно и без ошибок

### datagram socket
* internet-socket
* “неподключаемый” (без установки логического соединения)
  + не открываете точку подключения, как при потоковых сокетах
  + строите пакет, прикрепляете к нему IP заголовок с адресом назначения и отсылаете
  + подключения не требуются
  + но можно подключиться connect()-ом при желании
* если отправите в сокет послания “1, 2”, то на другой стороне они появятся в порядке “1, 2” (перепроверить)
* ненадёжны
  + если посылаете дейтаграмму, она может и появиться
  + она может появиться не в нужном порядке
  + если появилась, то данные в пакете будут переданы без ошибок
* хорошая скорость
* используют IP маршрутизацию
* используют User Datagram Protocol, UDP (не TCP)
* нужны, когда недоступен TCP стек или когда допускаeтся потеря нескольких пакетов (игры, звук, видео)

## Инкапсуляция данных
![Screenshot from 2024-04-06 17-00-28](https://github.com/akostrik/IRC-fork/assets/22834202/2697a7e0-024c-48ff-a907-42b82bd057a1)
1. пакет рождён
2. пакет завёрнут (инкапсулирован) в заголовок (реже и хвостик) протоколом первого уровня (скажем, TFTP)
3. вся штучка (включая TFTP заголовок) инкапсулируется следующим протоколом (скажем, UDP)
4. снова следующим (IP)
5. снова протоколом аппаратного (физического) уровня (скажем, Ethernet)

Layered Network Model (aka “ISO/OSI”), 7 уровней:
* Приложений (telnet, ftp, ...)
* Представления
* Сессии
* Транспортный (TCP, UDP)
* Сетевой (IP и маршрутизация)
* Данных
* Физический (уровень доступа к сети, Ethernet, wi-fi, ...)

## IP	адрес
* система маршрутизации сетей Internet Protocol Version 4, IPv4
  + адреса из четырёх байт (192.0.02.111)
  + все сайты используют IPv4
* IPv6
  + `192.0.2.33` = `::ffff:192.0.2.33`
* подсеть
  + первая часть IP является сетевой частью IP адреса, а остальная это часть хоста
* порт ≈ местный адрес для соединений
  + чтобы компьютер обрабатывал входящую почту и web сервисы, т.е. различить их на компьютере с одним IP адресом
  + IP ≈ адрес отеля, номер порта ≈ номер комнаты
  + различные интернет сервисы имеют различные известные номера портов (HTTP 80, telnet 23, SMTP 25, ...)
* порядок	байт	
  + Big-Endian / Little-Endian
  + Network Byte Order (=Big-Endian) / Host Byte Order
  + строя пакет или заполняя структуры вам нужно быть уверенным, что числа построены в Порядке Байтов Сети
    - прогоняете данные через **функции (системные вызовы) установки порядка байт сети** (`htons()`, `htonl()`, `ntohs()`, `ntohl()`)
    - преобразовать числа в Порядок Байтов Сети перед тем, как послать
    - в Порядок Байтов Хоста, когда они оттуда придут
    - если вам захочется поработать с плавающей запятой, см. Сериализация
* IPv6 имеет адрес и номер порта также, как IPv4
* параграф "4. Прыжок из IPv4 в IPv6" нам не нужен, там описывают, как менять код, предназначенный для IPv4, чтобы они работал с IPv6

## Частные (или отключённые) сети	
* **брандмауэр** скрывает сеть от остального мира
* брандмауэр транслирует “внутренние” IP адреса во “внешние” (известные всему миру) IP адреса с помощью процесса Network Address Translation (NAT)
* начинающему не нужно беспокоиться об NAT
* Например:
  + есть брандмауэр, два статичных IPv4 адреса, выделенных мне DSL компанией, и 7 компьютеров в сети
  + два компьютера не могут иметь одинаковый адрес
  + они находятся в частной сети с выделенными для неё 24 миллионами IP адресов
  + если я вхожу в удалённый компьютер - он говорит мне, что я вошёл с `192.0.2.33`, публичного IP, который мне выделил мой провайдер
  + если я спрашиваю у моего локального компьютера его адрес, он отвечает `10.0.0.5`
  + брандмауэр + NAT транслируют один IP адрес в другой
* номера частных сетей: наиболее частые `10.x.x.x` и `192.168.x.x`, менее распространены `172.y.x.x`, y от 16 до 31
* адреса `10.x.x.x` зарезервированны, используются в полностью отключённых сетях либо за брандмауэрами
  + сетям за брандмауэром не нужно быть одной из этих зарезервированных сетей, но обычно это так
* IPv6 тоже имеет частные сети
  + начинаются с `fdxx:`
  + NAT и IPv6 как правило не смешиваются, однако у вас будет столько адресов, что NAT больше не
понадобится
  + можно выделить для себя адреса в сети, которая не доступна снаружи

## системные вызовы ()
* ядро выполняет всю работу за вас
* здесь системные вызовы расположены примерно в том порядке, в каком к ним надо обращаться в программе
* здесь в примерах не включена проверка на ошибки
* я добавила в этот раздел функции, которые есть в задании (но нет в книге): setsockopt(), getsockname(), getprotobyname(), gethostbyname(), freeaddrinfo(), inet_addr(), inet_ntoa(), signal(), sigaction(), lseek(), fstat(), fcntl(), poll() or equivalent
* я не включила в этот раздел функции, которых нет в задании (но есть в книге): sendto(), recvfrom(), shutdown(), getpeername(), gethostname()

### getaddrinfo()
* поиск имён DNS и служб
* заполняет структуры
* параметры
  + node: имя или IP адрес хоста
  + service: номер порта или имя службы, например `http`, `ftp`, `telnet`, `smtp` (см. `/etc/services`)
  + hints: указывает на struct addrinfo, которую вы уже заполнили нужной информацией
* возвращает указатель на связанный список результатов
  
Пример: сервер, слушает порт 3490 вашего IP адреса (в действительности “слушания” или установки сети не происходит, просто заполняются структуры)
```
int             status;
struct addrinfo hints;
struct addrinfo *servinfo;                                          // укажет на результат

memset(&hints, 0, sizeof hints);
hints.ai_family   = AF_UNSPEC;                                      // мне всё равно IPv4 либо IPv6
hints.ai_socktype = SOCK_STREAM;                                    // потоковый сокет TCP
hints.ai_flags    = AI_PASSIVE;                                     // назначить структурам сокета адрес моего локального хостазаписать мой IP для меня

if ((status = getaddrinfo(NULL, "3490", &hints, &servinfo)) != 0) {
  fprintf(stderr, "getaddrinfo error: %s\n", gai_strerror(status));
  exit(1);
}
// servinfo указывает на связанный список из 1 или более struct addrinfo
freeaddrinfo(servinfo);                                             // освободить связанный список
```  
Пример: клиент, подсоединяется к серверу “www.example.net” порт 3490 (это не настоящее подключение, а заполнение структур)
```
int             status;
struct addrinfo hints;
struct addrinfo *servinfo; // укажет на результат

memset(&hints, 0, sizeof hints);
hints.ai_family   = AF_UNSPEC;
hints.ai_socktype = SOCK_STREAM;
status = getaddrinfo("www.example.net", "3490", &hints, &servinfo); // готовьтесь к соединению
// servinfo указывает на связанный список из 1 или более struct addrinfo
```
Пример: программа выводит IP адреса заданного в командной строке хоста (1 аргумент)
```
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <netinet/in.h>

int main(int argc, char *argv[]) {
  struct addrinfo hints, *res, *p;
  int             status;
  char            ipstr[INET6_ADDRSTRLEN];

  memset(&hints, 0, sizeof hints);
  hints.ai_family    = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  if ((status = getaddrinfo(argv[1], NULL, &hints, &res)) != 0) {
    fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(status));
    return 2;
  }
  printf("IP addresses for %s:\n\n", argv[1]);
  for(p = res;p != NULL; p = p->ai_next) {
    void *addr;
    char *ipver;
    if (p->ai_family == AF_INET) { // в IPv4 и IPv6 поля разные
      struct sockaddr_in *ipv4 = (struct sockaddr_in *)p->ai_addr;
      addr = &(ipv4->sin_addr);
      ipver = "IPv4";
    } else {
      struct sockaddr_in6 *ipv6 = (struct sockaddr_in6 *)p->ai_addr;
      addr = &(ipv6->sin6_addr);
      ipver = "IPv6";
    }
    inet_ntop(p->ai_family, addr, ipstr, sizeof ipstr); // перевести IP в строку и распечатать
    printf(" %s: %s\n", ipver, ipstr);
  }
  freeaddrinfo(res);
  return 0;
}
```
### socket() получить дескриптор файла
* параметры
  + IPv4 или IPv6 (`PF_INET` или `PF_INET6`) (`AF_INET` почти то же, что `PF_INET`)
  + потоковый или дейтаграммный (`SOCK_STREAM` или `SOCK_DGRAM`)
  + TCP или UDP (0 или вызвать getprotobyname() и выбрать нужный протокол, `tcp` or `udp`)
* возвращает ескриптор сокета, который вы можете позже использовать в системных вызовах (-1 в случае ошибки, errno = код ошибки)
Пример: взять данные из результатов вызова getaddrinfo() и передать их socket()
```
int             s;
struct addrinfo hints, *res;                                     // полагаем, что “hints" уже заполнена

getaddrinfo("www.example.com", "http", &hints, &res);            // проверить выход getaddrinfo() на ошибки
s = socket(res->ai_family, res->ai_socktype, res->ai_protocol);  // и просмотреть список "res" на действительный элемент, не полагаясь на то, что это первый
```
### bind()	На каком я порте?
* если сервер будет слушать (`listen())` входные подключения на специальном порте, то он связывает сокет с портом на вашей локальной машине
* для клиента bind() не нужно, клиент выполняет только `connect()`
* параметры:
  + файловый дескриптор сокета, возвращенный socket()-ом
  + указатель на struct sockaddr, которая содержит информацию о вашем адресе, а именно, порт и IP address
  + длина этого адреса в байтах
* в случае ошибки возвращает -1 и устанавливает errno
* бывают времена, когда вы абсолютно не можете вызвать bind(). Если вы подключаетесь (connect()) к уделённой машине и вас не заботит номер локального порта (как в случае с telnet, где вам нужен только удалённый порт), вы можете просто вызвать connect(), он проверит, подключён ли порт и, если необходимо, вызовет bind() и подключит свободный локальный порт (???)
* выбирать номер порта от 1024 до 65535
* Иногда перезапускаете сервер и bind() сбоит “Адрес уже занят”. Это кусочек подключавшегося сокета до сих пор висит в ядре и занимает порт. Вы можете подождать около минуты, пока он очистится, или добавить в программу код, позволяющий использовать порт повторно:
```
int yes=1;
if (setsockopt(listener,SOL_SOCKET,SO_REUSEADDR,&yes,sizeof(int)) == -1) {
  perror("setsockopt");
  exit(1);
}
```
Пример: привяжем сокет к порту 3490 хоста, на котором выполняется программа
```
struct addrinfo hints, *res;
int             sockfd;

// заполнить адресные структуры с помощью getaddrinfo():
memset(&hints, 0, sizeof hints);
hints.ai_family   = AF_UNSPEC;
hints.ai_socktype = SOCK_STREAM;
hints.ai_flags    = AI_PASSIVE;                                      // привязаться к IP хоста, на котором выполняется
getaddrinfo(NULL, "3490", &hints, &res);                             // если хотите соединиться с отдельным локальным IP адресом, укажите IP адрес в первом аргументе getaddrinfo()
sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol); // создать сокет
bind(sockfd, res->ai_addr, res->ai_addrlen);                         // связать с портом, полученным из getaddrinfo()
```
Пример: в книжке помечен как устаревший, но в нашем задании есть эти функции. Заполняем sockaddr_in вручную. Это специфично для IPv4, дляIPv6 тоже можно так делать. 
```
int                sockfd;
struct sockaddr_in my_addr;

sockfd                  = socket(PF_INET, SOCK_STREAM, 0);
my_addr.sin_family      = AF_INET;
my_addr.sin_port        = htons(MYPORT);                     // порядок байт сети
my_addr.sin_addr.s_addr = inet_addr("10.12.110.57");         // можно INADDR_ANY
memset(my_addr.sin_zero, '\0', sizeof my_addr.sin_zero);
bind(sockfd, (struct sockaddr *)&my_addr, sizeof my_addr);
```
### connect()
### listen()
### accept()
### send() и recv()
### close()	
### setsockopt()
### getsockname()
### getprotobyname()
### gethostbyname()
### freeaddrinfo()
### inet_addr()
### inet_ntoa()
### signal()
### sigaction()
### lseek()
### fstat()
### fcntl()
### poll() or equivalent

## типы данных, применяемых в интерфейсе сокетов (можно не читать)
### 1. дескриптор сокета: `int`
### 2. addrinfo
  + для подготовки адресных структур сокета для дальнейшего использования
  + для поиска имён хоста и службы
  + это одна из первых вещей, вызываемых при создании соединения
  + `getaddrinfo()` возвращает указатель на новый связанный список этих структур
  + обычно нет нужды заполнять эти структуры, достаточно вызвать `getaddrinfo()`, всё нужное там
  + код, написанный до изобретения структуры addrinfo, вставлял все эти принадлежности вручную
```
struct addrinfo {
  int                ai_flags;          // AI_PASSIVE, AI_CANONNAME, т.д.
  int                ai_family;         // AF_INET, AF_INET6, AF_UNSPEC
  int                ai_socktype;       // SOCK_STREAM, SOCK_DGRAM
  int                ai_protocol;       // используйте 0 для"any"
  size_t             ai_addrlen;        // размер ai_addr в байтах
  struct sockaddr    *ai_addr;          // struct sockaddr_in или _in6
  char               *ai_canonname;     // полное каноническое имя хоста
  struct addrinfo    *ai_next;          // связанный список, следующий
};
```
### 3. sockaddr
  + содержит адресную информацию для многих типов сокетов
  ```
  struct sockaddr {
    unsigned         short sa_family;   // семейство адресов, у нас AF_INET (IPv4) или AF_INET6
    char             sa_data[14];       // адрес назначения и номер порта для сокета
  };
  ```
### 4. sockaddr_in
  + для работы со структурой sockaddr
  + только для IPv4
  + “in” = “Internet”
  + указатель на sockaddr_in может быть приведен к указателю на sockaddr, и наоборот
  + `connect()` требует `struct sockaddr*`, вы можете пользоваться структурой `sockaddr_in`
  + позволяет обращаться к элементам адреса сокета
  + `sin_zero` включён для расширения длины структуры до длины sockaddr
  + `sin_zero` должен быть обнулён функцией `memset()`
  + `sin_family` соответствует `sockaddr.sa_family` и должен быть установлен в `AF_INET`
  + используйте htons()
```
struct sockaddr_in {
  short int          sin_family;        // семейство адресов, AF_INET
  unsigned short int sin_port;          // номер порта
  struct in_addr     sin_addr;          // интернет адрес
  unsigned char      sin_zero[8];       // размер как у struct sockaddr
};
```
### 5. in_addr
  + исторически обоснованная структура
  + только для IPv4
  + интернет адрес
  + даже если ваша система до сих пор использует `union` для структуры `in_addr`, вы всё равно можете ссылаться на 4-байтный IP адрес благодаря #define-ам
```
struct in_addr {
  uint32_t           s_addr;            // 32-битный int
};
```
Пример 
```
struct sockaddr_in ina;
ina.sin_addr.s_addr;                    // 4-байтный IP адрес (в Порядке Байтов Сети) 
```
### 6. sockaddr_in6
  + только для IPv6
```
struct sockaddr_in6 {                  
  u_int16_t       sin6_family;          // семейство адресов, AF_INET6
  u_int16_t       sin6_port;            // номер порта, Порядок Байтов Сети
  u_int32_t       sin6_flowinfo;        // потоковая информация IPv6
  struct in6_addr sin6_addr;            // (???) адрес IPv6, можно записать глобальную переменную in6addr_any, можно использовать макрос IN6ADDR_ANY_INIT (≈ INADDR_AN` для IPv4) 
  u_int32_t       sin6_scope_id;        // Scope ID
};
```
### 7. in6_addr
  + только для IPv6
```
struct in6_addr {
  unsigned char  s6_addr[16];            // адрес IPv6
}
```
### 8. sockaddr_storage **(?)**
  + содержит обе IPv4 и IPv6 структуры
  + подобна `sockaddr`, только больше
  + если не знаете наперёд, каким адресом загружать структуру `sockaddr` (IPv4 или IPv6), то передайте sockaddr_storage и приведите к нужному типу
  + через `ss_family`посмотреть - это `AF_INET или AF_INET6, и, когда нужно, привести к `sockaddr_in` или `sockaddr_in6`
```
struct sockaddr_storage{
  sa_family_t    ss_family;              // семейство адресов
  char           __ss_pad1[SS_PAD1SIZE]; // выравнивание длины, зависит от реализации, проигнорируйте
  int64_t        __ss_align;
  char           __ss_pad2[SS_PAD2SIZE];
};
```

### преобразования формы записи IP-адреса
+ упаковывать IP-адрес в long оператором `<<` нет нужды
+ `inet_pton()`
  - pton = presentation to network
  - преобразовывать строковые IP адреса в их двоичное представление
  - преобразует `10.12.110.57` или `2001:db8:63b3:1::3490` в `struct in_addr` либо `struct in6_addr` в зависимости от параметра AF_INET или AF_INET6
  - возвращает -1 при ошибке и 0 если произошла какая-то путаница (надо проверять)
+ `inet_ntop()`
  - nton = network to presentation
  - например, чтобы напечатать `struct in_addr`в представлении `10.12.110.57`
  - например, чтобы напечатать `struct in6_addr`в представлении `2001:db8:63b3:1::3490`
+ эти функции работают только с цифровыми IP адресами, не с именами хостов для DNS серверов типа `www.example.com` (для `www.example.com` использовать `getaddrinfo()`)
+ устаревшие функции, не работают с IPv6: `inet_addr()`, `inet_aton()`, `inet_ntoa()` (но все три указаны в нашем задании)

`10.12.110.57` -> `struct sockaddr_in`  
```
struct sockaddr_in  sa;        
inet_pton(AF_INET, “192.0.2.1”, &(sa.sin_addr));   
```
`2001:db8:63b3:1::3490` -> `struct sockaddr_in6`
```
struct sockaddr_in6 sa6;   
inet_pton(AF_INET6, “2001:db8:63b3:1::3490”, &(sa6.sin6_addr));
```
`struct sockaddr_in` -> `10.12.110.57`
```
charIPv4            ip4[INET_ADDRSTRLEN];                    // место для строки IPv4
struct sockaddr_in  sa;                                      // предположительно чем-то загружено
inet_ntop(AF_INET, &(sa.sin_addr), ip4, INET_ADDRSTRLEN);
printf(“The IPv4 address is: %s\n”, ip4);
```
`struct sockaddr_in6` -> `2001:db8:63b3:1::3490`
```
charIPv6            ip6[INET6_ADDRSTRLEN];                   // место для строки IPv6
struct sockaddr_in6 sa6;                                     // предположительно чем-то загружено
inet_ntop(AF_INET6,&(sa6.sin6_addr), ip6, INET6_ADDRSTRLEN);
printf(“The IPv6 address is: %s\n”, ip6);
```
