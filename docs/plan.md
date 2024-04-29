## нерешённые проблемы: давайте ориентироваться только на сабжект + irssi, иначе мы не успеем
* USER: может ли быть uName и rName пустым? (levensta регистрирует пользлвтаеля даже если rName = "")
* правильно ли работают:
  + USER a 0 * a, USER a 0 * a
  + PRIVMSG alice,alice hello
  + JOIN #сhannel,#сhannel
  + JOIN #channel, JOIN #channel
  + MODE #ch
* PRIVMSG: verify that is **fully functional with different parameters** (checklist)
  + RCF2812 : The <target> parameter may also be a host mask (#<mask>)
  + The server will only send the PRIVMSG to those who have a host matching the mask
  + The mask MUST have at least 1 (one) "." in it and no wildcards following the last ".". This requirement exists to prevent people sending messages to "#*" or "$*", which would broadcast to all users
  + Wildcards are the '*' and '?'  characters.  This extension to the PRIVMSG command is only available to operators.
* По PRIVMSG 
  + Исправить ошибку с верхним регистром, никнеймы могут быть и в верхнем регистре
  + У получателя paul24 высвечивается 
* PRIVMSG paul24 ::Gello gow
  + Убрать двойное двоеточие
  + У получателя должен высвечиваться не ник получателя, а ник отправителя (от кого сообщение пришло)
* JOIN when a user joins a server you have to greed him with a welcome message
* JOIN 0 = PART со всех каналов
* JOIN #channel в irssi после этого все сообщения идут только в этот канал ?
* PART #channel1,#channel2
* PART прощальные сообщения
* PART если прощального сообщения нет, то уведомление об отключении с канала (всем посылать и отключившемуся и текущим пользователям)
* KICK, а потом INVITE того же пользователя, выходит сообщшение INVITE chel2 #chan1` `443 chel2 #chan1 :is already on channel` Хотя этого человека уже исключили из канала
* INVITE с только что созданного канала - пишет, что приглашенный пользователь уже есть на сервере INVITE Bakyt1 #chan1, 443 Bakyt1 #chan1 :is already on channel
* MODE #myChannel +l 999999999999999`
* MODE +l -1
* MODE устанвливает один параметр за раз, например `MODE -t` должна работать, а `MODE -tpk` нет, нормально ли это?
* MODE: админа обознгачать "nick@" всякий раз, когда он ассоциируется с каналом
* QUIT допроверить 
* PRIVMSG, KICK, PART: сообщение пользователю начинается с двоеточия, Первый символ (двоеточие) у получателя не отображается (видимо на сервере надо удалить его)
  + у получателя не отображается в irssi или в nc? если в irssi, то скорее всего это irssi его удаляет, но сервер должен посылать с двлоеточием
* JOIN, MODE, KICK, QUIT, PRIVMSG: отправляются всем пользователям канала
* send() вернула число, меньшее длины буфера, т.е. отправила не весь буфер (у Марии это есть)
* sned() вернула 0? это значит клиент пропал или нет?
* to send partial commands, ensure that other connections still run fine (checklist)
* kill a client, check that the server is still operational for the other connections and for any new incoming client (checklist)
* kill a nc with just half of a command sent. Check again that the server is not in an odd state or blocked. (checklist)
* some test with the IRC client and nc at the same time (checklist)
* receiving partial data (checklist)
* low bandwidth (checklist)
* Stop a client (^-Z) connected on a channel. Then flood the channel using another client. When the client is live again, all stored commands should be processed normally. Also, check for **memory leaks** during this operation. (checklist)
* Check that a regular user does not have privileges to do operator actions. Then test with an operator. All the channel operation commands should be tested. (checklist)
* проверка утечек памяти, особено в случае нестандартных ситуаций
* PASS must be send before any other paquet, yell a message only if the user is registered (nick and password was successfuly completed) and refuse the connection at this moment (you can do it on bad PASS directly if you wish) https://ircgod.com/docs/irc/to_know/
* Un channel "exclusif" à deux users cmd PRIVMSG + nickname - должно рабоать как переписка между ними (?)
* Флаг +k, пароль установлен, при попытке джоин пишет что нельзя войти, но всё рано присоединяет к каналу
* segfault когда прерываем nc

## проблемы второй срочности
* to anwser a client for status update (nick change, mode, etc…), the packet must be formed like this: `:<nickname>@<username>!<hostname> <COMMAND> <arg>\r\n` нужно ли?
* PASS если неправильный пароль никакого сообщения?
* NOTICE выдавать сообщение с каким-нибудь цветом
* команда чтобы посмотреть кто является оператором канала - вроде это MODE
* команда чтобы посмотреть кто есть на канале - вроде это MODE
* OPER регистрации клиента самого себя в качестве оператора канала
* Clients connecting from a host which name is longer than 63 characters are registered using the host (numeric) address instead of the host name
* **wireshark** / netcat / a custom **proxy** etc… permet de **voir en raw ce qui est send entre ton client et ton serveur**, easily debug your server, gives you the ability to check how already existing one behaves 
  + альтерантива: https://github.com/LiveOverflow/PwnAdventure3/blob/master/tools/proxy/proxy_part9.py.
* IRC channels
  + [netsplit.de Search](https://netsplit.de/channels/ ) - Searches 563 different networks.
  + [mibbit Search](https://search.mibbit.com) - Searches networks listed [here](https://search.mibbit.com/networks).
  + [KiwiIRC Search](https://kiwiirc.com/search) - Searches 318 different networks.
  + [#ubuntu](https://wiki.ubuntu.com/IRC/ChannelList)@Libera.Chat - Official Ubuntu support channel. ([rules](https://wiki.ubuntu.com/IRC/Guidelines))
* сервер для тестов
  + https://oragono.io/
  + irssi: `/connect irc.freenode.net`, `/join #ubuntu,#ubuntuforums,#ubuntu+1`
  + freenode
  + des serveurs qui étaient déjà installés sur l’application Hexchat
  + irc.ircgod.com:6667/6697
  + Don’t use libera.chat as a testing server, it use a lot of ircv3.0 features
* если есть лишние аргументы, сервер их игнорирует?
* проблема valgrind: PASS NICK USER JOIN QUIT

## решённые проблемы
* при установке лимита на количество пользователей на канале (команда MODE #channel +l 20) изменяется топик канала, а не описание MODE
* если вводится неправильное количество аргументов (например для команды MODE #channel +l 20 необходимо 4 аргумента) - нет уведомления об ошибке, что количество аргументов неправильное.
* PASS  с пробелом, то видимо программа считает что есть второй аргумент и ошибка не выводится
* получается зарегистрировать пользователя с подобным ником или с пустым ником и отправлять дубликату сообщения
* для пароля нам нужен ответ - если пароль не введен, то мы должны ответ ERR_NEEDMOREPARAMS
* Я залогинился под неправильным паролем с двоих терминалов (слева и справа клиентские NC) - ввел потом ники, наименование пользователя, и получилось сообщение отправить
* PRIVMSG без аргументов segfault 
* в команде KICK должен быть также либо комментарий либо сообщение о том кто сделал KICK, это сообщение тоже должно быть разослано на всех участников канала 
* KICK прощальные сообщения
* двоеточие это начало сообщения и можно это воспринимать как аргумент текста
* если ошибка в нике, то он не имеет права попробовать с другим ником, мы его сразу отключаем
* NOTICE: verify that is **fully functional with different parameters** (checklist). The difference between NOTICE and PRIVMSG is that automatic replies MUST NEVER be sent in response to a NOTICE message
* одновременно два сервера не запускались
* JOIN напечатать всех участников канала

## Инфо
* https://github.com/levensta/IRC-Server
* https://github.com/marineks/Ft_irc
* https://github.com/miravassor/irc
* https://modern.ircdocs.horse/
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
  + можно взять названия каналов только с `#` https://discord.com/channels/774300457157918772/785407578972225579/922447406606458890
