## Разобраться или доделать
* одна команда может оказаться разбитой на несколько сообщений или нет ?
  + Кажется у Бориса не может
  + TCP is a streaming protocol, not a message protocol
    - The only guarantee is that you send n bytes, you will receive n bytes in the same order.
    - You might send 1 chunk of 100 bytes and receive 100 1 byte recvs, or you might receive 20 5 bytes recvs.
    - You could send 100 1 byte chunks and receive 4 25 byte messages.
    - **You must deal with message boundaries yourself**. (у нас messages boundaries это `\n`, правильно?)
  + Из RFC 1459: В предоставление полезной 'non-buffered' сети IO для клиентов и серверов, каждое соединение из которых является частным 'input buffer', в котором результируются большинство полученного, читается и проверяется. Размер буфера 512 байт, используется как одно полное сообщение, хотя обычно оно бывает с разными командам. Приватный буфер проверяется после каждой операции чтения на правильность сообщений. Когда распределение с многослойными сообщениями от одного клиента в буфере, следует быть в качестве одного случившегося, клиент может быть 'удален'.
  + `com^Dman^Dd` (* use ctrl+D **to send the command in several parts**: `com`, then `man`, then `d\n`). You have to first aggregate the received packets in order to rebuild it
  + https://stackoverflow.com/questions/108183/how-to-prevent-sigpipes-or-handle-them-properly
  + у Ахмеда так считвыается: `ssize_t bytes = recv(fd, buff, sizeof(buff) - 1 , 0);`, почему минус 1?
  + то же самое в littleServer из книжки: `numbytes = recv(sockfd, buf, MAXDATASIZE-1, 0)`
  + подводные камни tcp-ip сокетамов: данные в tcp-ip стеке могут появляться не все сразу, а кусками. Если клиент послал данные с помощью одной функции send(), это совсем не значит, что они могут быть приняты одной функцией recv(). https://forum.sources.ru/index.php?showtopic=43245
  + в проекте mariia они делают как-то "tokenize the buffer line by line"
* должна ли наша PRIVMSG понимать маски и особые формы записи?
  + `PRIVMSG #*.edu :NSFNet is undergoing work, expect interruptions` Сообщение для всех пользователей, сидящих на хосте, попадающим под маску *.edu
  + Борис проверяет `"${receiver}"`зачем-то
  + в дискорде пишут надо только `#` https://discord.com/channels/774300457157918772/785407578972225579/922447406606458890
  + Параметр <receiver> может быть маской хоста (#mask) или маски сервера ($mask)
    - Cервер будет отсылать PRIVMSG только тем, кто попадает под серверную или хост-маску
    - Маска должна содержать в себе как минимум одну "." - это требование вынуждаеит пользователей отсылать сообщения к "#*" или "$*", которые уже потом рассылаются всем пользователям; по опыту, этим злоупотребляет большое количество пользователей
    - В масках используются '*' и '?', это расширение команды PRIVMSG доступно только IRC-операторам
* `JOIN #foo,&bar fubar` вход на канал #foo, используя ключ "fubar" и на канал &bar без использования ключа - я пока ничего не сделала насчёт `&`, надо ли?
* я сделала `MODE` для установки одного параметра за раз, например `MODE -t` должна работать, а `MODE -tpk` нет, нормально ли это? 
* **много команд или ответов на команды не указаны в сабджекте, но без них клиент работать не будет** (**какие именно команды необходимы?**)
* The command MUST either be a valid IRC command or **a three (3) digit number represented in ASCII text** - то есть возможно надо понимать просто сообщения-числа? Возможно, это легче, чем строкию.
* когда мы в irssi, то как бы попадаем на канал и там остаёмся, нам тоже так надо?
* wireshark не понимаю, как использоать, вроде это полезная утилита
  + Wireshark permet de **voir en raw ce qui est send entre ton client et ton serveur**. Dans le meilleur des cas tu utilises un serv irc dans docker et un client irc. Puis tu sniff avec wireshark pour regarder ce qui est send. Puis tu t'adapte.
  + Use wireshark / a custom **proxy** etc… to inspect communication between your reference server (or your server) and you your client
  + **a proxy**, in our case we used a modified version of this proxy: https://github.com/LiveOverflow/PwnAdventure3/blob/master/tools/proxy/proxy_part9.py. Having a proxy allows you to easily debug your server and also gives you the ability to check how already existing one behaves.
  + wireshark, netcat etc
* `valgrind`, закрытие сокетов

## Наладить связть с irssi
* Irssi envoie la commande CAP en plus de NICK et USER (discord)
  + https://scripts.irssi.org/scripts/cap_sasl.pl
  + CAP LS [version] to discover the available capabilities on the server
  + CAP REQ to blindly request a particular set of capabilities
  + CAP END. Upon receiving either a CAP LS or CAP REQ command, the server MUST not complete registration until the client sends a CAP END command to indicate that capability negotiation has ended. This allows clients to request their desired capabilities before completing registration.
  + https://ircv3.net/specs/extensions/capability-negotiation.html
  + irssi: il faut envoyer au nouveau client connecté la RPL 001 pour que irssi comprenne qu'il est bien co
* аня на личном компе:
  + скачала irssi https://doc.ubuntu-fr.org/irssi
  + запустила наш сервер на порту 6667
  + ввела в терминале `irssi`
  + в самом irssi `/connect 0 -tls_pass 2`
* альтернативы irssi: kvirc, bitchx (хвалят), ircnet (respecte completement (ou presque) les rfc), ngircd, libera chatь HexChat, gamja, sic, Quassel, Yaaic, relay.js, Circe, Smuxi, Konversation, Revolution IRC, IRC for Android, Iridium, IRC Vitamin, anope, oragono, irc omg, Bv

## Выбрать сервер для тестов (чтобы сравнивать с нашим)
* Don’t use libera.chat as a testing server, it’s a great irc server but it use a lot of ircv3.0 features, instead use self hostable one (ngirc, oragono etc…) you can even use our one, irc.ircgod.com:6667/6697
* server is 90% of the time built according to oragono irc server https://oragono.io/
* irssi: `/connect irc.freenode.net`, `/join #ubuntu,#ubuntuforums,#ubuntu+1`
* freenode
* liberachat
* pour tester j’ai pris des serveurs qui étaient déjà installés sur l’application Hexchat

  
## Протестировать наш сервер + выбранный клиент, настоящий сервер + выбранный клиент
* **[rfc2812 messages client -> server](https://datatracker.ietf.org/doc/html/rfc2812)**
* rfc 2813 messages server -> server, нам не нужно
* rfc 1459 устарел
* https://modern.ircdocs.horse/
* [IRCv3 Specifications](https://ircv3.net/irc/)
* `RCv3 extensions` надеюсь нам не нужно 
* Using your reference client with your server must be **similar to using it with any official IRC server**. (subject)
* Имя канала обязательно начинается с `#`? Я пока сделала так, но кажется это неправильно, вроде бы `#` это маска хоста
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
* Le serveurs n'a le droit qu'a un seul send() par client pour chaque poll() ou select() (?)
* you only are allowed to do 1 (one) send() per select()
*  Si la channel n'est pas créer tu peux ignorer la clé (comme quand le mode +k n'est pas activé au final)
* остановилась на сообщении Ah. On a pris libera chat comme référence depuis le début lol.

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
