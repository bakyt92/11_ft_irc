## мелкие вопросы
* в irssi то после команды `join #ch` все сообщения идут только в этот канал, нам тоже так надо?
* точно ли нам не нужен ip-6
* `valgrind`, закрытие сокетов
* должна ли наша PRIVMSG понимать маски и особые формы записи? например `PRIVMSG #*.edu :NSFNet is undergoing work, expect interruptions`

## нерешённые проблемы

- когда делаем с канала KICK, а потом делаем INVITE того же пользователя - выходит сообщение 
`INVITE chel2 #chan1` `443 chel2 #chan1 :is already on channel` Хотя по идее этого человека уже исключили из канала, и например после INVITE и отправки сообщений в канал - этот клиент сообщения не получает
- для пароля нам нужен ответ - если пароль не введен, то мы должны ответ направлять ERR_NEEDMOREPARAMS, пока что ответа нет
- я сейчас проверил, в функции PASS есть проверка на количество аргументов, но если мы вводим "PASS " с пробелом, то видимо программа считает что есть второй аргумент и ошибка не выводится
- видимо Ник не дублируется, но получается зарегистрировать пользователя с подобным ником или с пустым ником и отправлять дубликату сообщения (по идее мы не должны регистрировать дубликата). сначала в правом терминале зарегистрировал ник  BaKyt и пользователя с именем Bakyt и потом в левом терминале - ник bakyt с другим регистром и пользователя bakyt, то есть вроде бы дубликат - не должен был регистрироваться дубликат. ну то есть как мне кажется - мы просто отдаем клиенту сообщение об ошибке но не возвращаем его на предыдущую стадию
- Я залогинился под неправильным паролем с двоих терминалов (слева и справа клиентские NC) - ввел потом ники, наименование пользователя, и получилось сообщение отправить
- и еще я получал segfault когда так под одним терминалом зашел и просто  PRIVMSG без аргументов вызвал
- надо только проверку на пароль добавить
- JOIN 0 - по документации это команда по аналогу PART со всех каналов
- PART #ch error de segmentation1. При исполнении команды PART можно удаляться из 2 и более каналов, у нас эта функция не работает (выкидывает segfault, хотя по коду у нас это вроде есть). Синтаксис клиента "PART #channel1,#channel2"
- При исполнении команды PART может быть прощальное сообщение, но если его ввести, то выходит segfault
- При исполнении команды INVITE даже с только что созданного канала - пишет, что приглашенный пользователь уже есть на сервере (хотя канал только что создали)
INVITE Bakyt1 #chan1
443 Bakyt1 #chan1 :is already on channel
- по команде PART - если прощального сообщения нет, то просто уведомление об отключении с канала посылается (я предлагаю всем посылать и отключившемуся и текущим пользователям)
- в команде KICK должен быть также либо комментарий либо сообщение о том кто сделал KICK. Я думаю что это сообщение тоже должно быть разослано на всех участников канала (пока что не реализовано)
- добавить, что делать, если send вернула число, меньшее длины буфера, т.е. отправила не весь буфер (у Марии это есть)
- что делать, если send вернула 0? это значит клиент пропал или нет?

## решённые проблемы
- при установке лимита на количество пользователей на канале (команда MODE #channel +l 20) изменяется топик канала, а не описание MODE.
- если вводится неправильное количество аргументов (например для команды MODE #channel +l 20 необходимо 4 аргумента) - нет уведомления об ошибке, что количество аргументов неправильное.

## Тесты irssi + наш сервер, irssi + настоящий сервер
* irssi `/connect 0 -tls_pass 2`
* сервер для тестов
  + https://oragono.io/
  + irssi: `/connect irc.freenode.net`, `/join #ubuntu,#ubuntuforums,#ubuntu+1`
  + freenode
  + des serveurs qui étaient déjà installés sur l’application Hexchat
  + irc.ircgod.com:6667/6697
  + Don’t use libera.chat as a testing server, it use a lot of ircv3.0 features
* **[rfc2812 messages client -> server](https://datatracker.ietf.org/doc/html/rfc2812)** (rfc 2813 server -> server, нам не нужно, rfc 1459 устарел)
* http://abcdrfc.free.fr/rfc-vf/rfc1459.html
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
* неправильнй пароль
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
* to anwser a client for status update (nick change, mode, etc…), the packet must be formed like this: `:<nickname>@<username>!<hostname> <COMMAND> <arg>\r\n`

## Инфо
* https://github.com/levensta/IRC-Server
* https://github.com/marineks/Ft_irc
* https://github.com/miravassor/irc
* https://masandilov.ru/network/guide_to_network_programming – гайд по сетевому программированию
* https://ncona.com/2019/04/building-a-simple-server-with-cpp/ – про сокеты и TCP-сервер на C++
* https://youtu.be/cNdlrbZSkyQ – еще немного про сокеты
* https://www.ibm.com/docs/en/i/7.3?topic=designs-example-nonblocking-io-select non-blocking I/O
* http://www.kegel.com/dkftpbench/nonblocking.html non-blocking I/O
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
* [Сетевое программирование от Биджа. Использование	Интернет Сокетов. (кратко)](https://github.com/bakyt92/11_ft_irc/blob/master/docs/book_sockets_short.md)   
* https://www.irchelp.org/
* https://ircgod.com/ (!)
* https://stackoverflow.com/questions/27705753/should-i-use-pass-before-nick-user/27708209#27708209
* [заметки Ани](https://github.com/akostrik/CPP_modules_42)
* из группы дискорд:
  + To test ipv6 you can use irssi and add -6 during the /connect
  + on gere ipv4 et ipv6, impossible de recup **l'addr ipv4**
  + Si la channel n'est pas créer tu peux ignorer la clé (comm quand le mode +k n'est pas activé au final)
  + **Oper name** is not the same thing as your nickname / username etc, oper is like using sudo -u
  + остановилась на сообщении c'est a dire "tu geres bien les users que tu as envoye a ton client lors du join" ?
