## План 
1. Найти ИРК рабочий и попробовать запустить https://github.com/AhmedFatir/ft_irc
2. Настя и вся команда - разберет серверную часть, все прочитаем гайд на 100 стр. 
3. Найти видео - разобраться в концепции

## Три основные части проекта:
1. Создайте все необходимые классы и методы для проекта.
2. Создайте сокет и обработайте сигналы сервера.
3. Используйте poll()функцию, чтобы проверить, произошло ли событие.
4. Если событие представляет собой новый клиент, зарегистрируйте его.
5. Если событие представляет собой данные зарегистрированного клиента, обработайте его.
6. Обработка различных команд IRC, таких как PASS, NICK, USER, JOIN, PART, TOPIC, INVITE, KICK, QUIT, MODE и PRIVMSG

## Неоходимые функции IRC:
Многопоточная архитектура для обработки одновременных клиентских подключений.  
Поддержка нескольких одновременных подключений.  
Создание и управление IRC-каналами.  
Аутентификация и регистрация пользователя.  
Рассылка сообщений всем пользователям канала.  
Обмен личными сообщениями между пользователями.  
Поддержка ников пользователей и названий каналов.  
Подключитесь к IRC-серверу.  
Присоединяйтесь к каналам и участвуйте в групповых беседах.  
Отправляйте и получайте сообщения.  
Изменить псевдоним пользователя.  
Отправляйте личные сообщения другим пользователям.  
Обработка различных команд IRC, таких как PASS, NICK, USER, JOIN, PART, TOPIC, INVITE, KICK, QUIT, MODE и PRIVMSG  

## Разобраться или доделать (важное)
* обработку префиксов, напрмимер `:Alice NICK Bob` Alice изменяет свой никнейм на Bob (`:Alice` это префикс)
  + https://www.lissyara.su/doc/rfc/rfc1459/
* одна команда может оказаться разбитой на несколько сообщений или нет ???
  + Кажется у Бориса это не учитывается
  + TCP is a streaming protocol, not a message protocol.
  + The only guarantee is that you send n bytes, you will receive n bytes in the same order.
  + You might send 1 chunk of 100 bytes and receive 100 1 byte recvs, or you might receive 20 5 bytes recvs.
  + You could send 100 1 byte chunks and receive 4 25 byte messages.
  + **You must deal with message boundaries yourself**.
* All I/O operations must be non-blocking - всё ли ок с этим у нас?
* `valgrind`
* Можно ли иметь однорвеменно пользователя с ником `Alice`и канал `Alice`? 
  
## Разобраться или доделать (не очень важное)
* должна ли наша команда PRIVMSG понимать маски и обобые случаи?
  + `:Alice PRIVMSG Bob :Hello are you receiving this message ?` Сообщение от Alice к Bob
  + `PRIVMSG Alice :yes I'm receiving it !receiving it !'u>(768u+1n) .br`                                Сообщение к Angel
  + `PRIVMSG jto@tolsun.oulu.fi :Hello !` Сообщение от клиента на сервер tolsun.oulu.fi с именем "jto";
  + `PRIVMSG $*.fi :Server tolsun.oulu.fi rebooting.` Сообщение ко всем, кто находится на серверах, попадающих под маску *.fi
  + `PRIVMSG #*.edu :NSFNet is undergoing work, expect interruptions` Сообщение для всех пользователей, сидящих на хосте, попадающим под маску *.edu
  + Борис проверяет подстроку `"${receiver}"`
* нужны ли где-то модификаторы `const`?
* у Бориса когда сервер получает данные из сокета, он их записывает в буфер размером 1024. После этого делает split буфера - разделяет (по разделителю пробелу) команду и аргументы, сохраняет это в векторе. И после этого проверяет каждый аргумент, не длиннее ли он 512 байт. 
Но тут https://www.lissyara.su/doc/rfc/rfc1459/ написано, что максимальная длина команды вместе с аргументами это 512.
* почему у Бориса буфер для получения команд unsigned char*, в этом есть какой-то смысл?
* почему у Борсиа send иногда без флагов, иногда с флагом MSG_NOSIGNAL
  + MSG_NOSIGNAL = requests not to send the SIGPIPE signal if an attempt to send is made on a stream-oriented socket that is no longer connected
  + don't generate a SIGPIPE signal if the peer has closed the connection
  + но почему у него иногда 0, иногда MSG_NOSIGNAL в send()?
* Я просто закоментировала `fcntl`, нормально ли это?
                               
## Протестировать нашу программу и реальный сервер
* [rfc1459](https://github.com/bakyt92/11_ft_irc/blob/master/docs/rfc1459.txt)
* [rfc2812](https://datatracker.ietf.org/doc/html/rfc2812)
* команду QUIT получает, даже если клиент не залогинен, а дргуие команды не получет в этой ситуации
* `PASS`, `pass` и `paSS` должны одинаково рабоатть?
* Нестандартные ситуации
  + неправильное имя канала
  + неправлиьнй пароль (по RCF 1459 и RCF 2812 не понятно!)
  + очень длинное собщение
  + пустая строка
  + `PRIVMSG alice,alice hello`
  + `nick   an   `
  + `nick '`
  + `com^Dman^Dd` (* use ctrl+D to send the command in several parts: `com`, then `man`, then `d\n`)
  + `PASS ` (с пробелом) должна отвечать "неправильный пароль" или "не хватает аргументов"?
  + `USER al 0 * Alice`, потом опять`USER al 0 * Alice`
  + Verify that the poll() is called every time before each accept, read/recv, write/send. After these calls, errno should not be used to trigger specific action (e.g. like reading again after errno == EAGAIN). (checklist)
  + The server can handle multiple connections at the same time. The server should not block. It should be able to answer all demands. Do some test with the IRC client and nc at the same time. (checklist)
  + Join a channel. Ensure that all messages from one client on that channel are sent to all other clients that joined the channel. (checklist)
  + Just like in the subject, try to send partial commands (checklist)
    - Check that the server answers correctly
    - With a partial command sent, ensure that other connections still run fine
  + Unexpectedly kill a client. Then check that the server is still operational for the other connections and for any new incoming client. (checklist)
  + Unexpectedly kill a nc with just half of a command sent. Check again that the server is not in an odd state or blocked. (checklist)
  + Stop a client (^-Z) connected on a channel. Then flood the channel using another client. The server should not hang. When the client is live again, all stored commands should be processed normally. Also, check for memory leaks during this operation. (checklist)
  + Verify that private messages (PRIVMSG) and notices (NOTICE) are **fully functional with different parameters**. (checklist)
  + Check that a regular user does not have privileges to do operator actions. Then test with an operator. All the channel operation commands should be tested (remove one point for each feature that is not working). (checklist)


## Просмотреть группу в дискорд

## Инфо
Используем клиент https://kiwiirc.com/nextclient/    
https://medium.com/@afatir.ahmedfatir/small-irc-server-ft-irc-42-network-7cee848de6f9  
https://modern.ircdocs.horse/   
https://irc.dalexhd.dev/index.html  
https://beej.us/guide/bgnet/html/  
https://rbellero.notion.site/School-21-b955f2ea0c1c45b981258e1c41189ca4   
https://www.notion.so/FT_IRC-c71ec5dfc6ae46509fb281acfb0e9e29?pvs=4  
https://www.youtube.com/watch?v=I9o-oTdsMgI (инструкция и видео)   
https://www.youtube.com/watch?v=gntyAFoZp-E (основы, что и в методичке, ток на английском)  
https://www.youtube.com/watch?v=I9o-oTdsMgI  
https://www.youtube.com/@edueventsmow/videos  
https://www.youtube.com/watch?v=I9o-oTdsMgI&list=PLUJCSGGiox1Q-QvHfMMydtoyEa1IEzeLe&index=1   
https://github.com/shuygena/intra42_ru_guides?tab=readme-ov-file#irc (много информации по проектам)  
https://github.com/mharriso/school21-checklists/blob/master/ng_5_ft_irc.pdf   
https://github.com/marineks/Ft_irc  
[Сетевое программирование от Биджа. Использование	Интернет Сокетов. (кратко)](https://github.com/bakyt92/11_ft_irc/blob/master/docs/book_sockets_short.md)   
[Аня разбирает код тут](https://github.com/akostrik/IRC-fork/blob/master/README.md)  
[Internet Relay Chat Protocol - RFC 1459](https://www.lissyara.su/doc/rfc/rfc1459)  
[RCF 2812](https://datatracker.ietf.org/doc/html/rfc2812)  

