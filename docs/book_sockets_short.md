# Сетевое программирование от Биджа. Использование интернет	сокетов.
https://github.com/bakyt92/11_ft_irc/blob/master/docs/bgnet_A4_rus.pdf  

## Socket
* как Unix-программа выполняет любую операцию ввода/вывода? чтением или записью в дескриптор файла
* дескриптор файла = целое число, связанное с открытым файлом
* файл = сетевое подключение, FIFO, конвейер, терминал, реальный файл на диске и всё, что угодно
* `socket()` возвращает дескриптор сокета
* `send(), recv()` ≈ `read(), write()`, но больше возможностей управления передачей данных

## stream socket
* internet-socket
* подключаемые двунаправленные потоки связи
* используется в telnet приложениях
* для загрузки страниц web браузерами по HTTP протоколу
  + если подключиться к web сайту по telnet-у и, напечатать “GET / HTTP/1.0”, получишь html
* The Transmission Control Protocol, TCP
* TCP обеспечивает появление ваших последовательно и без ошибок

## datagram socket
* internet-socket
* “неподключаемые” (без установки логического соединения)
  + не открываете точку подключения, как при потоковых сокетах
  + строите пакет, прикрепляете к нему IP заголовок с адресом назначения и отсылаете
  + подключения не требуются
  + но можно подключиться connect()-ом при желании
* Если отправите в сокет послания “1, 2”, то на другой стороне они также появятся в порядке “1, 2”
* ненадёжны
  + если посылаете дейтаграмму, она может и появиться
  + она может появиться не в нужном порядке
  + если появилась, то данные в пакете будут переданы без ошибок
* хорошая скорость
* используют IP маршрутизацию
* используют User Datagram Protocol, UDP (не TCP)
* нужны, когда недоступен TCP стек или когда допускаeтся потеря нескольких пакетов (игры, звук, видео)

## raw sockets
* internet-socket

## Инкапсуляция данных
![Screenshot from 2024-04-06 17-00-28](https://github.com/akostrik/IRC-fork/assets/22834202/2697a7e0-024c-48ff-a907-42b82bd057a1)
1. пакет рождён
2. пакет завёрнут (“инкапсулирован”) в заголовок (реже и хвостик) протоколом первого уровня (скажем,
TFTP)
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
  + строя пакет или заполняя структуры вам нужно быть уверенным, что числа построены с Порядке Байтов Сети
    - прогоняете данные через функции **установки порядка байт сети** (htons(), htonl(), ntohs(), ntohl())
    - преобразовать числа в Порядок Байтов Сети перед тем, как послать
    - в Порядок Байтов Хоста, когда они оттуда придут
    - если вам захочется поработать с плавающей запятой, см. Сериализация
* IPv6 имеет адрес и номер порта также, как IPv4
  
## типы данных, применяемых в интерфейсе сокетов
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
  + только для IPv4
  + интернет адрес (исторически обоснованная структура)
  + даже если ваша система до сих пор использует `union` для структуры `in_addr`, вы всё
равно можете ссылаться на 4-байтный IP адрес благодаря #define-ам
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
  struct in6_addr sin6_addr;            // адрес IPv6
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
