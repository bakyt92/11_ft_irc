## может оказаться одна команда оказаться разбитой на несколько сообщений ?
* у Бориса если команда длинне 512 символов, от просто отрасывает лишнее
* IRC использует транспортный протокол TCP и криптографический TLS (опционально)
* TCP is a streaming protocol, not a message protocol
    - The only guarantee is that you send n bytes, you will receive n bytes in the same order
    - You might send 1 chunk of 100 bytes and receive 100 1 byte recvs, or you might receive 20 5 bytes recvs
    - You could send 100 1 byte chunks and receive 4 25 byte messages
    - **You must deal with message boundaries yourself**
    - You can't rely on "getting the whole message" at once, or in any predictable size of pieces
    - You have to build a protocol or use a library which lets you identify the beginning and end of your application specific messages
    - You should read data coming back into a buffer and either prefix the message with a message length or use start/end message delimiters to determine when to process the data in the read buffer
* RFC 2812: IRC messages are always lines of characters terminated with a CR-LF pair, and these messages SHALL NOT exceed 512 characters in length, counting all characters including the trailing CR-LF. Thus, there are 510 characters maximum allowed for the command and its parameters. **There is no provision for continuation of message lines**. 
* Ахмед: `ssize_t bytes = recv(fd, buff, sizeof(buff) - 1 , 0);`, почему минус 1?
* littleServer из книжки: `numbytes = recv(sockfd, buf, MAXDATASIZE-1, 0)`
* у марии тоже size - 1
* данные в tcp-ip стеке могут появляться не все сразу, а кусками. Если клиент послал данные с помощью одной функции send(), это не значит, что данные могут быть приняты одной функцией recv(). https://forum.sources.ru/index.php?showtopic=43245
* в проекте mariia есть "tokenize the buffer line by line"
* Le serveurs n'a le droit qu'a **un seul send() par client pour chaque poll() ou select()** 

## должна ли наша PRIVMSG понимать маски и особые формы записи?
  + `PRIVMSG #*.edu :NSFNet is undergoing work, expect interruptions` Сообщение для всех пользователей, сидящих на хосте, попадающим под маску *.edu
  + Борис проверяет `"${receiver}"`зачем-то
  + Параметр <receiver> может быть маской хоста (#mask) или маски сервера ($mask)
    - Cервер будет отсылать PRIVMSG только тем, кто попадает под серверную или хост-маску
    - Маска должна содержать в себе как минимум одну "." - это требование вынуждаеит пользователей отсылать сообщения к "#*" или "$*", которые уже потом рассылаются всем пользователям; по опыту, этим злоупотребляет большое количество пользователей
    - В масках используются '*' и '?', это расширение команды PRIVMSG доступно только IRC-операторам

## Programs that use non-blocking I/O: 
* https://www.ibm.com/docs/en/i/7.3?topic=designs-example-nonblocking-io-select
* http://www.kegel.com/dkftpbench/nonblocking.html
* every function returns immediately, i.e. all the functions in such programs are nonblocking
* Instead, they use the "state machine" technique
* ...
* How to query the available data on a socket:
  + Non-bloking sockets
  + select()/poll()

## сигналы
  + `com^Dman^Dd` (* use ctrl+D **to send the command in several parts**: `com`, then `man`, then `d\n`). You have to first **aggregate the received packets in order to rebuild it**
  + https://stackoverflow.com/questions/108183/how-to-prevent-sigpipes-or-handle-them-properly
  + EOF processing (Control-D) is handled in canonical mode; it actually means 'make the accumulated input available to read()'; if there is no accumulated input (if you type Control-D at the beginning of a line), then the read() will return zero bytes, which is then interpreted as EOF by programs. Of course, you can merrily type more characters on the keyboard after that, and programs that ignore EOF (or run in non-canonical mode) will be quite happy 
https://stackoverflow.com/questions/358342/canonical-vs-non-canonical-terminal-input
  + CTRL+D посылает 0 байт только если при пустой строке мы это вводим
  + если вводим сначала символы и потом CTRL+D, то это своего рода сигнал отправить текущие символы из командной строки
  + если брать наш пример `PRIV ^D MSG ^D Nickname Hello! \r \n`, то команда CTRL+D сначала отправит <PRIV EOF> - это видимо 6 байт, а потом <MSG EOF> - 5 байт, а не ноль
  + Наверное в сабджекте имелось также в виду, что мы игнорируем входящие символы ^D. Получается это ^D равен четырем, то есть char c == 4
    - https://stackoverflow.com/questions/75676419/eof-and-ctrl-d
    - https://www.physics.udel.edu/~watson/scen103/ascii.html
  + writing to non-responding socket will cause a SIGPIPE and make my server crash
     - `send(...MSG_NOSIGNAL)` = write() without SIGPIPE
  + Ctrl+D is a keyboard input that typically represents the EOF character
    - When entered at the beginning of a line in a terminal, it signals the end of input to the terminal
    - It's commonly used to indicate the end of input when reading from stdin
    - It doesn't directly raise a signal like SIGPIPE
    - it's processed by the terminal or the program reading from stdin

## irssi
* https://scripts.irssi.org/scripts/cap_sasl.pl
* https://ircv3.net/specs/extensions/capability-negotiation.html
* https://hub.docker.com/_/irssi
* аня на личном компе:
  + запустила наш сервер на порту 6667
  + ввела в терминале `irssi`
  + в самом irssi `/connect 0 -tls_pass 2`
* альтернативы irssi: kvirc, bitchx (хвалят), ircnet (respecte completement (ou presque) les rfc), ngircd, libera chat, HexChat, gamja, sic, Quassel, Yaaic, relay.js, Circe, Smuxi, Konversation, Revolution IRC, IRC for Android, Iridium, IRC Vitamin, anope, oragono, irc omg, Bv, brew
* Irssi commands: accept die knock notice sconnect unload action disconnect knockout notify script unnotify admin echo  lastlog op scrollback unquery alias eval layout oper server unsilence away exec links part servlist upgrade ban flushbuffer list ping set uptime beep foreach load query sethost userhost bind format log quit silence ver cat hash lusers quote squery version cd help map rawlog squit voice channel hilight me recode stats wait clear ignore mircdcc  reconnect statusbar wall completion info mode redraw time wallops connect invite motd rehash toggle who ctcp      ircnet  msg reload topic whois cycle ison names resize trace whowas dcc join nctcp restart ts window dehilight kick netsplit rmreconns unalias deop kickban network rmrejoins unban devoice kill nick save unignore  

## Выбрать сервер для тестов (чтобы сравнивать с нашим)
* https://oragono.io/
* irssi: `/connect irc.freenode.net`, `/join #ubuntu,#ubuntuforums,#ubuntu+1`
* freenode
* liberachat
* des serveurs qui étaient déjà installés sur l’application Hexchat
* irc.ircgod.com:6667/6697
* Don’t use libera.chat as a testing server, it use a lot of ircv3.0 features
* hésite pas à test avec notre IRC https://github.com/Assxios/ft_irc.git
* https://github.com/hallainea/ft_irc
  
## Протестировать наш сервер + выбранный клиент, настоящий сервер + выбранный клиент
* **[rfc2812 messages client -> server](https://datatracker.ietf.org/doc/html/rfc2812)** (rfc 2813 server -> server, нам не нужно, rfc 1459 устарел)
* **wireshark** / netcat / a custom **proxy** etc… permet de **voir en raw ce qui est send entre ton client et ton serveur**, easily debug your server, gives you the ability to check how already existing one behaves 
  + альтерантива: https://github.com/LiveOverflow/PwnAdventure3/blob/master/tools/proxy/proxy_part9.py.
* https://modern.ircdocs.horse/
* [IRCv3 Specifications](https://ircv3.net/irc/)
* `RCv3 extensions` надеюсь нам не нужно 
* Using your reference client with your server must be **similar to using it with any official IRC server**. (subject)
* Имя канала обязательно начинается с `#`? Я пока сделала так, но вроде бы `#` это маска хоста
  + дискорд: можно взять названия каналов только с `#` https://discord.com/channels/774300457157918772/785407578972225579/922447406606458890
  + RFC: "@" is used for secret channels
  + RFC: "*" for private channels
  + RFC: "=" for others (public channels)
* наша упрощённая версия НЕ во всём должна работать как настощий сервер (вроде нам не нужны маски, у нас только один сервер, ...)
*  Un channel "exclusif" à deux users cmd PRIVMSG + nickname
  + то есть для каналов из двух пользователей отправлять не по имени канала, а по имени пользователя?
* если есть лишние аргументы, сервер всегда их просто игнорирует?
* есть ли какие-то ограничения на пароль?
* IRC channels.
  + [netsplit.de Search](https://netsplit.de/channels/ ) - Searches 563 different networks.
  + [mibbit Search](https://search.mibbit.com) - Searches networks listed [here](https://search.mibbit.com/networks).
  + [KiwiIRC Search](https://kiwiirc.com/search) - Searches 318 different networks.
  + [#ubuntu](https://wiki.ubuntu.com/IRC/ChannelList)@Libera.Chat - Official Ubuntu support channel. ([rules](https://wiki.ubuntu.com/IRC/Guidelines))
* Как выглядит параметр `mode`, когда сервер его показывает ?
  + https://stackoverflow.com/questions/12886573/implementing-irc-rfc-how-to-respond-to-mode
* `MODE #myChannel -lktt`
* `MODE #myChannel +ltk` 5 myTopic myPass
* неправильное имя канала
* неправильнй пароль (по RCF 1459 и RCF 2812 не понятно!) 
* что отвечает сервер на `MODE #myChannel +l 999999999999999`
* очень длинное собщение
* команды QUIT, PASS, NICK, USER выполняет, даже если клиент не залогинен, а дргуие команды не выполняет в этой ситуации
* Ограничения на имя канала такие же, как на имя пользователя?
* пустая строка
* Надо ли удалять двоеточие из текста сообщения `:Alice PRIVMSG Bob :Hello`
* `PRIVMSG alice,alice hello`
* `nick   alice    `
* `nick '`
* `PASS  ` (с пробелом)
* `PASS 3`,`PASS 2`
* `PASS`, `pass` и `paSS` должны одинаково рабоатть?
* `USER al 0 * Alice`, потом опять`USER al 0 * Alice`
* `Alice` и `alice` это один и тот же ник? (судя по коду бориса, да)
* `MyChannel` и `myChannel` это один и тот же канал?
* `JOIN #myChannel,#myChannel`
* `MODE +l -1`
* The server should not block. It should be able to answer all demands. Do some test with the IRC client and nc at the same time. (checklist)
* Just like in the subject, try to send partial commands (checklist)
  + With a partial command sent, ensure that other connections still run fine
* PASS must be send before any other paquet, yell a message only if the user is registered (nick and password was successfuly completed) and refuse the connection at this moment (you can do it on bad PASS directly if you wish) https://ircgod.com/docs/irc/to_know/
* Unexpectedly kill a client. Then check that the server is still operational for the other connections and for any new incoming client. (checklist)
* Unexpectedly kill a nc with just half of a command sent. Check again that the server is not in an odd state or blocked. (checklist)
* Stop a client (^-Z) connected on a channel. Then flood the channel using another client. The server should not hang. When the client is live again, all stored commands should be processed normally. Also, check for memory leaks during this operation. (checklist)
* Verify that private messages (PRIVMSG) and notices (NOTICE) are **fully functional with different parameters**. (checklist)
* Check that a regular user does not have privileges to do operator actions. Then test with an operator. All the channel operation commands should be tested. (checklist)
* канал `news` уже существует, а ты создаёшь ещё один канал `news`
* можно ли иметь однорвеменно пользователя с ником `Alice`и канал `Alice`
* `PASS JOIN` `NICK JOIN` `USER JOIN 0 * JOIN` `JOIN #JOIN` etc
* более 15 параметров
* с двоеточием кажется много особых случаев, я пока это вообще не учитывала
* в целом, **многое не понятно именно с поведением сервера** (какие точно сообщения в каком случае отправлять, как реагировать на нестандартные ситуации), надо сделать очень много тестов, по документу rfc1459 далеко не всё понятно
* сигналы
* что делать, если единственный админ канала покидает его? канал остаётся без админа?
* receiving partial data (subject)
* low bandwidth (subject)
* https://github.com/opsec-infosec/ft_irc-tester
* https://github.com/markveligod/ft_irc
* when a user joins a server you have to greed him with a welcome message
* ставить @ перед ником админа?
* Verify absolutely every possible error and issue (receiving partial data, low bandwidth, ...) (checklist)

## Читаю группу дискорд:
* кто-то предлагает использовать openssl, чтобы не хранить пароль в октрытом виде
* в 2021 г. пишут про `:WiZ!jto@tolsun.oulu.fi NICK Kilroy`
* If a client send a CAP command, ignore it, don’t throw an error
* To test ipv6 you can use irssi and add -6 during the /connect
* IRC default port is 6667
* Add **MSG_NOSIGNAL** as a 4th argument for send, it will prevent your programm from crashing under certain condition
  + Genre le client il fait legit connect();send();exit() ducoup il est plus rapide que toi. Et tu te tape des signal sigpipe
* **Oper name** is not the same thing as your nickname / username etc, oper is like using sudo -u
* that to anwser a client for status update (nick change, mode, etc…), the packet must be formed like this: `:<nickname>@<username>!<hostname> <COMMAND> <arg>\r\n`
* on gere et ipv4 et ipv6, impossible de recup **l'addr ipv4**
* Deja vu un segfault dans SSL_write car ce dernier essaye d'acceder à l'addr 0x30, or cette derniere n'est pas mappé (on pense ca vient de sslptr)
  + ca arrive vraiment ULTRA rarement, genre 1 fois sur 400, et dans des conditions VRAIMENT extreme, genre en l'occurence switch h24 entre 3g/4g/wifi et tenter de se reco à chaque fois avec dans le meme temps plein d'user qui se deco reco au meme tick etc... ?
  + l'addr mdr, 0x30, c'est l'ascii pour 0 genre on (je) pense que ca peut pas etre une coincidence quoi
* tout les messages doivent finir par **\r\n**
* Si la channel n'est pas créer tu peux ignorer la clé (comme quand le mode +k n'est pas activé au final)
* Operator password is not the same thing as server password
* je me tape des residus "fantomes" de memoire..
* Si j'ai bien compris, on doit renvoyer plusieurs réponses numériques (genre RPL_WHOISUSER RPL_WHOISCHANNELS, et RPL_WHOISSERVER par exemple) pour ensuite afficher les différentes infos sur un utilisateur de notre serveur IRC.
Or, nos fonctions de commande (de mon groupe j'entends) renvoient seulement un int, celui correspondant à un "numeric reply". Doit-on renvoyer plusieurs numeric reply ou bien un seul ? Si c'est le dernier, comment faire dans le cas de WHOIS pour afficher toutes les infos sur un utilisateur avec un seul "numeric reply" ?
* остановилась на сообщении c'est a dire "tu geres bien les users que tu as envoye a ton client lors du join" ?

## мелкие вопросы
* точно ли нам не нужен ip-6
* `valgrind`, закрытие сокетов
* в irssi то после команды `join #ch` все сообщения идут только в этот канал, нам тоже так надо?
* что делать, если админ покинул чат?

## Инфо
* **most public IRC servers don't usually set a connection password**
* https://medium.com/@afatir.ahmedfatir/small-irc-server-ft-irc-42-network-7cee848de6f9  
* https://modern.ircdocs.horse/   
* https://irc.dalexhd.dev/index.html  
* https://beej.us/guide/bgnet/html/  
* https://rbellero.notion.site/School-21-b955f2ea0c1c45b981258e1c41189ca4   
* https://www.notion.so/FT_IRC-c71ec5dfc6ae46509fb281acfb0e9e29?pvs=4  
* https://www.youtube.com/watch?v=I9o-oTdsMgI (инструкция и видео)   
* https://www.youtube.com/watch?v=gntyAFoZp-E (основы, что и в методичке, ток на английском)  
* https://www.youtube.com/watch?v=I9o-oTdsMgI  
* https://www.youtube.com/@edueventsmow/videos  
* https://www.youtube.com/watch?v=I9o-oTdsMgI&list=PLUJCSGGiox1Q-QvHfMMydtoyEa1IEzeLe&index=1   
* https://github.com/shuygena/intra42_ru_guides?tab=readme-ov-file#irc (много информации по проектам)  
* https://github.com/mharriso/school21-checklists/blob/master/ng_5_ft_irc.pdf   
* https://github.com/marineks/Ft_irc
* https://github.com/miravassor/irc
* [Сетевое программирование от Биджа. Использование	Интернет Сокетов. (кратко)](https://github.com/bakyt92/11_ft_irc/blob/master/docs/book_sockets_short.md)   
* https://www.irchelp.org/
* https://ircgod.com/ (!)
* https://stackoverflow.com/questions/27705753/should-i-use-pass-before-nick-user/27708209#27708209
* [заметки Ани](https://github.com/bakyt92/11_ft_irc/edit/master/ann/README.md) 
