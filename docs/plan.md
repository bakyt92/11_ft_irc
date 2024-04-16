## План 
1. Найти ИРК рабочий и попробовать запустить https://github.com/AhmedFatir/ft_irc
2. Используем клиент https://kiwiirc.com/nextclient/
3. Настя и вся команда - разберет серверную часть, все прочитаем гайд на 100 стр. 
4. Найти видео - разобраться в концепции

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

## Доделать
* обработку префиксов, напрмимер `:Alice NICK Bob` Alice изменяет свой никнейм на Bob (`:Alice` это префикс)
  + https://www.lissyara.su/doc/rfc/rfc1459/
* одна команда может оказаться разбита на несколько сообщений
  + Кажется у Бориса это не учитывается
  + TCP is a streaming protocol, not a message protocol.
  + The only guarantee is that you send n bytes, you will receive n bytes in the same order.
  + You might send 1 chunk of 100 bytes and receive 100 1 byte recvs, or you might receive 20 5 bytes recvs.
  + You could send 100 1 byte chunks and receive 4 25 byte messages.
  + **You must deal with message boundaries yourself**.
  
## Разобраться или доделать (не очень важные вопросы)
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
* много инфо тут [rfc1459](https://github.com/bakyt92/11_ft_irc/blob/master/docs/rfc1459.txt) 
* команду QUIT получает, даже если клиент не залогинен, а дргуие команды не получет в этой ситуации
* `PASS`, `pass` и `paSS` должны одинаково рабоатть?
* Нестандартные ситуации
  + неправильное имя канала
  + непhавлиьнй пароль
  + очень длинное собщение
  + пустая строка
  + `PRIVMSG alice,alice hello`
  + `nick   an   `
  + `nick '`
  + `com^Dman^Dd` (* use ctrl+D to send the command in several parts: `com`, then `man`, then `d\n`)
  
## Инфо
Используем клиент https://kiwiirc.com/nextclient/    
https://medium.com/@afatir.ahmedfatir/small-irc-server-ft-irc-42-network-7cee848de6f9  
https://modern.ircdocs.horse/   
https://irc.dalexhd.dev/index.html  
https://beej.us/guide/bgnet/html/  
https://github.com/shuygena/intra42_ru_guides?tab=readme-ov-file#irc (много информации по проектам)  
https://rbellero.notion.site/School-21-b955f2ea0c1c45b981258e1c41189ca4   
https://www.notion.so/FT_IRC-c71ec5dfc6ae46509fb281acfb0e9e29?pvs=4  
https://www.youtube.com/watch?v=I9o-oTdsMgI (инструкция и видео)   
https://www.youtube.com/watch?v=gntyAFoZp-E (основы, что и в методичке, ток на английском)  
https://www.youtube.com/watch?v=I9o-oTdsMgI  
https://www.youtube.com/@edueventsmow/videos  
https://simple.wikipedia.org/wiki/List_of_Internet_Relay_Chat_commands   
https://github.com/mharriso/school21-checklists/blob/master/ng_5_ft_irc.pdf   
https://www.youtube.com/watch?v=I9o-oTdsMgI&list=PLUJCSGGiox1Q-QvHfMMydtoyEa1IEzeLe&index=1   
[Краткое содержание: Сетевое программирование от Биджа. Использование	Интернет Сокетов.](https://github.com/bakyt92/11_ft_irc/blob/master/docs/book_sockets_short.md)   
[Аня разбирает код тут](https://github.com/akostrik/IRC-fork/blob/master/README.md)  
[Internet Relay Chat Protocol - RFC 1459](https://www.lissyara.su/doc/rfc/rfc1459)  

