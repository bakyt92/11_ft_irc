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
* `valgrind`, закрытие сокетов
                               
## Читаю группу дискорд:
* кто-то предлагает использовать openssl, чтобы не хранить пароль в октрытом виде
* в 2021 г. пишут про `:WiZ!jto@tolsun.oulu.fi NICK Kilroy`
* l'implementation qui respecte completement (ou presque) les rfc est celle d'ircnet
* votre serveur il fonctionne avec irssi wechat hexchat etc... ?
* pour tester j’ai pris des serveurs qui étaient déjà installés sur l’application Hexchat
* une peerclass irc ptdr (не поняла совсем)
* If a client send a CAP command, ignore it, don’t throw an error
* To test ipv6 you can use irssi and add -6 during the /connect
* IRC default port is **6667**, use it, it’s annoying to specify it while testing in a defense (when using irssi for example, specifying 6667 every time at the end is boring)
* Add **MSG_NOSIGNAL** as a 4th argument for send, it will prevent your programm from crashing under certain condition
  + Genre le client il fait legit connect();send();exit() ducoup il est plus rapide que toi. Et tu te tape des signal sigpipe
  + pour ca que faut normalement utiliser select pour lire
* **Oper name** is not the same thing as your nickname / username etc, oper is like using sudo -u
* ircd.tar.gz is just a basic tcp server, not an irc server, it’s not useful
*  Use wireshark / a custom **proxy** etc… to inspect communication between your reference server (or your server) and you your client
*  you MUST have to simplify the project by a LOT: **a proxy**, in our case we used a modified version of this proxy: https://github.com/LiveOverflow/PwnAdventure3/blob/master/tools/proxy/proxy_part9.py. But if you feel like, you can use wireshark netcat etc, but quite annoying to set up / use in my opinion. Having a proxy allows you to easily debug your server and also gives you the ability to check how already existing one behaves.
*  that to anwser a client for status update (nick change, mode, etc…), the packet must be formed like this: `:<nickname>@<username>!<hostname> <COMMAND> <arg>\r\n`
*  on gere et ipv4 et ipv6, impossible de recup **l'addr ipv4**
*  Deja vu un segfault dans SSL_write car ce dernier essaye d'acceder à l'addr 0x30, or cette derniere n'est pas mappé (on pense ca vient de sslptr), ca arrive vraiment ULTRA rarement, genre 1 fois sur 400, et dans des conditions VRAIMENT extreme, genre en l'occurence switch h24 entre 3g/4g/wifi et tenter de se reco à chaque fois avec dans le meme temps plein d'user qui se deco reco au meme tick etc... ? ce qui nous casse les pieds c'est l'addr mdr, 0x30, c'est l'ascii pour 0 genre on (je) pense que ca peut pas etre une coincidence quoi
*  tout les messages doivent finir par **\r\n**
*  остановилась на сообщении Ouaip j'ai jamais réussi à recevoir un NJOIN de ngircd

## Протестировать нашу программу и реальный сервер
* **[rfc2812](https://datatracker.ietf.org/doc/html/rfc2812)** concerns clients request, rfc 2813 server request, rfc 1459 is an old version of 2812
* rfc 2810, 2811, **2812**, 2813 переопределяют rfc 1459, надо брать новые (2813 is for multiserver)
* https://modern.ircdocs.horse/
* [IRCv3 Specifications](https://ircv3.net/irc/)
* Имя канала обязательно начинается с `#`? Я пока сделала так, но кажется это неправильно, вроде бы `#` это маска хоста
* le join du rfc 2812 et pour les msg client to server et celui du 2813 est pour les msg server to server. pareil pour user, nick et mode
* наша упрощённая версия НЕ во всём должна работать как настощий сервер (вроде нам не нужны маски, у нас только один сервер, ...)
* You have to choose one of the **IRC clients as a reference**. Your reference client will be used during the evaluation process. (subject)
  + **отсюда https://github.com/kvirc/KVIrc скачать kvirc и через cmake компилировать**
    - `# mkdir build && cd build`
    - `# cmake ..`
    - `# make install`
  + Your reference client must be able to connect to your server without encountering any error
  + As for the client the one that we choose almost every time is irrsi. A CLI irc client. We also went for revolutionIRC, hexchat and kiwirc for testing purposes.
  + Don’t use libera.chat as a testing server, it’s a great irc server but it use a lot of ircv3.0 features, instead use self hostable one (ngirc, oragono etc…) you can even use our one, irc.ircgod.com:6667/6697
  + server is 90% of the time built according to oragono irc server https://oragono.io/
  + sur mac tu peux faire un brew install ircd
  + [Textual](https://www.codeux.com/textual/) - Very customizable, ZNC integration, iCloud sync ($4.99). ([source](https://github.com/Codeux-Software/Textual)) `macOS`
  + [LimeChat](http://limechat.net/mac/) - One window for multiple servers, keyboard shortcuts, fast & stable. ([source](https://github.com/psychs/limechat)) `macOS` `iOS`
  + [HexChat](https://hexchat.github.io) - Based on XChat, easy to use, spell check & multiple languages. ([source](https://github.com/hexchat/hexchat)) `Windows` `macOS` `Linux`
  + [gamja](https://sr.ht/~emersion/gamja/) - A simple IRC web client. ([source](https://git.sr.ht/~emersion/gamja)) `Web`
  + [Kiwi IRC](https://kiwiirc.com) - Powerful modern IRC messenger for the web. ([source](https://github.com/kiwiirc/kiwiirc), [demo](https://kiwiirc.com/nextclient/)) `Web`
  + [CIRC](https://flackr.github.io/circ/) - Uses the chrome.sockets APIs to connect directly to IRC servers without needing a proxy. ([source](https://github.com/flackr/circ)) `Chrome`
  + [Quassel](https://quassel-irc.org) - Distributed (clients can attach to and detach from a central core that stays permanently online. ([source](https://github.com/quassel/quassel)) `Linux` `macOS` `Windows`
  + [Yaaic](https://www.yaaic.org) - Multi-server/channel support, SASL support, Smooth channel scrolling / swiping. ([source](https://github.com/pocmo/Yaaic)) `Android`
  + [relay.js](https://github.com/Fauntleroy/relay.js) - Focuses on making IRC less intimidating and easier to use. `Web`
  + [Circe](https://github.com/emacs-circe/circe) - For use in Emacs, sane defaults. `Emacs`
  + [Smuxi](https://smuxi.im) - User-friendly, based on GNOME / GTK+. ([source](https://github.com/meebey/smuxi)) `Linux` `Windows` `macOS`
  + [KvIRC](http://www.kvirc.net) - Free, portable, based on Qt GUI toolkit. ([source](https://github.com/kvirc/KVIrc)) `Linux` `macOS` `Windows`
  + [Konversation](https://konversation.kde.org) - User-friendly client built on the KDE Platform. ([source](https://github.com/KDE/konversation)) `Linux`
  + [sic](https://tools.suckless.org/sic/) - **S**imple **I**RC **c**lient - a terminal client in less than 250 lines of C. `Linux`
  + [irssi](https://irssi.org) - Terminal client, multi-protocol friendly for module authors, GPLv2. `Linux` `macOS` `Cygwin` `BSD`
  + [Revolution IRC](https://github.com/MCMrARM/revolution-irc) - Feature-full, actively maintained Android IRC client. `Android`
  + [AdiIRC](https://adiirc.com) - Never has a client offered such granular settings for every aspect of the IRC experience. ([features](https://dev.adiirc.com/projects/adiirc/wiki/Features), [screenshots](https://dev.adiirc.com/projects/adiirc/wiki/Screenshots)) `Windows` `WINE`
  + [IRC for Android™](https://www.countercultured.net/android/) - Android/Chrome OS client for power users, with ZNC built-ins, notification logic, reliable DCC, keybinds for hardware keyboards, etc. `Android` `ChromeOS`
  + [Iridium](https://appcenter.elementary.io/com.github.avojak.iridium/) - Friendly IRC client built in Vala and GTK, designed for elementary OS. ([source](https://github.com/avojak/iridium)) `Linux`
  + [IRC Vitamin](https://play.google.com/store/apps/details?id=com.todoartedigital.chuecamobile.irc.vitamin) - Simple, fast and easy access to multiple IRC networks. `Android`
  + ngircd
* если есть лишние аргументы, сервер всегда их просто игнорирует?
* Using your reference client with your server must be **similar to using it with any official IRC server**. (subject)
* IRC channels.
  + [netsplit.de Search](https://netsplit.de/channels/ ) - Searches 563 different networks.
  + [mibbit Search](https://search.mibbit.com) - Searches networks listed [here](https://search.mibbit.com/networks).
  +  [KiwiIRC Search](https://kiwiirc.com/search) - Searches 318 different networks.
  + [#ubuntu](https://wiki.ubuntu.com/IRC/ChannelList)@Libera.Chat - Official Ubuntu support channel. ([rules](https://wiki.ubuntu.com/IRC/Guidelines))
* Как выглядит параметр `mode`, когда сервер его показывает ?
  + https://stackoverflow.com/questions/12886573/implementing-irc-rfc-how-to-respond-to-mode
* `MODE #myChannel -lktt`
* `MODE #myChannel +ltk` 5 myTopic myPass
* неправильное имя канала
* неправильнй пароль (по RCF 1459 и RCF 2812 не понятно!) 
* что отвечает сервер на `MODE #myChannel +l 999999999999999`
* очень длинное собщение
* команды QUIT, PASS, NICK, USER выаолняет, даже если клиент не залогинен, а дргуие команды не выполняет в этой ситуации
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

## Инфо
* Используем клиент https://kiwiirc.com/nextclient/
  + есть ещё такие варианты: limechat
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
* [Сетевое программирование от Биджа. Использование	Интернет Сокетов. (кратко)](https://github.com/bakyt92/11_ft_irc/blob/master/docs/book_sockets_short.md)   
* https://www.irchelp.org/

