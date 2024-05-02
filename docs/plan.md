## Понять, как должен вести себя сервер
* NICK проверить, точно ли нужно :Nickname collision KILL
* PRIVMSG alice,alice hello
* PRIVMSG alice h1 h2 h3 h4 h5 h6 h7 h8 h9 h10 h11 h12 h13 h14 h15 **B** 
* PRIVMSG alice,bob,carol,david,eve,françois Hello - у нас лимит 5 адресатов, это нормально? **B** 
* PRIVMSG alice,alice,alice,alice,alice,alice **B** 
* NOTICE alice,alice Hello **B** 
* JOIN #channel, JOIN #channel
* **JOIN #channel в irssi после этого все сообщения по умолчанию идут в этот канал ?**
* INVITE с только что созданного канала - пишет: приглашенный пользователь уже есть на сервере **B** 
  + sends the user a message asking them if they want to join you in the indicated channel
  + то есть клиент не попадает сразу на канал
  + как клиент должен выразить своё согласие?
  + клиент должен сделать JOIN?
* INVITE на несуществующий канал **B** 
* MODE #ch
* MODE #channel +l 999999999999999`
* MODE #channel +l -1
* MODE Check that a regular user does not have privileges to do operator actions. Then test with an operator. All the channel operation commands should be tested. (**checklist**)
* MODE +t +t +t
* MODE +k 22, MODE +k 22
* TOPIC должна рассылать всем какое-то сообщение?
* TOPIC #channel должно отвечать "недостаточно параметров" или устанавливать пустой топик?
* TOPIC #channel ":" 
* WHOIS alice (до регистрации)
* test with irssi and nc at the same time (checklist)
* протестиовать с valgrind все функции с несуществущими каналами и пользователями
* протестировать с valgrind все случаи ошибок, которые есть в коде

## ПРОТЕСТИРОВАНО на IRSSI 
 * JOIN #сhannel,#сhannel
* USER a 0 * ""  - no command USER on IRSSI
* USER "" 0 * ""  - no command USER on IRSSI
* USER a 0 * a, USER a 0 * a  - no command USER on IRSSI
* PASS c неправильным паролем
* JOIN #ch1,ch2,ch3,ch4,ch5,ch6 - у нас лимит 5 каналов, это нормально? <== решили игнорировать, это нормальное поведение. 
* **B** PRIVMSG если между двумя пользователями есть канал на двоих, то все личные сообщения попадают туда ? <== в канал попадают значения с хэштегом #, в личные сообщения попадают сообщения на ник (без хэштега)
* **B** PRIVMSG без параметров, а также все команды без параметров <= по всем командам есть ошибка 451. Проверил. 

## решённые проблемы
* сначала NICK, потом PASS - некритична последовательность, только важно выполнение 3 условий (наличие ника, юзернейма, совпадение пароля)
* KICK, а потом INVITE того же пользователя, выходит сообщшение INVITE chel2 #chan1` `443 chel2 #chan1 :is already on channel` Хотя этого человека уже исключили из канала - РАБОТАЕТ (БАКЫТ)
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
* segfault PASS NICK USER JOIN - kill nc - SIGINT server
* QUIT
* проблема valgrind: PASS NICK USER JOIN QUIT
* to send partial commands, ensure that other connections still run fine (checklist)
* kill a client, check that the server is still operational for the other connections and for any new incoming client (checklist)
* kill a nc with just half of a command sent. Check again that the server is not in an odd state or blocked. (checklist)
* receiving partial data (checklist)
* проверка утечек памяти, особено в случае нестандартных ситуаций
* PART #channel1,#channel2
* PART прощальные сообщения
* PART если прощального сообщения нет, то уведомление об отключении с канала (всем посылать и отключившемуся и текущим пользователям)
* memory leak: PASS 2, NICK a, USER a 0 * a, JOIN #ch, PART #ch
* JOIN 0 = PART со всех каналов
* PRIVMSG У получателя paul24 высвечивается `paul24 ::Gello gow`, у получателя должен высвечиваться не ник получателя, а ник отправителя (от кого сообщение пришло), также  KICK, PART
* PRIVMSG У получателя paul24 высвечивается `paul24 ::Gello gow`, убрать двойное двоеточие
* send() вернула число, меньшее длины буфера, т.е. отправила не весь буфер
* PRIVMSG: никнеймы могут быть и в верхнем регистре
* TOPIC #ch myTopic - меняет топик
* MODE #ch +t : делает так, что только админ может менять топик
* MODE #ch -t : любой продписанный на канал может менят топик
* JOIN с паролем
* в каналах сообщения из NC в IRSSI не поступают
* MODE Флаг +k, пароль установлен, при попытке джоин пишет что нельзя войти, но всё рано присоединяет к каналу
* * Настройка setsockopt касается только первого сокета, который мы создаём, чтобы приниматть новых клиентов. После своего появления, каждый из клиентов создаёт сам себе сокет для сообщений. Сервер делает только accept этого сокета. И для этих клиентских сокетов в проектах пиров не вызывается setsockopt
  + If the listen queue is empty of connection requests and O_NONBLOCK is not set on the file descriptor for the socket, accept() shall block until a connection is present. If the listen() queue is empty of connection requests and O_NONBLOCK is set on the file descriptor for the socket, accept() shall fail and set errno to [EAGAIN] or [EWOULDBLOCK].
* удалять неиспользумые данные каналов, данные клинетов и poll - вроде бы работает
* WHOIS почему-то выдает I send buf to fd=5 : [401 :moscow No such nick_4\r\n318moscow :End of WHOIS list\r\n]
* https://docs.google.com/document/d/1LWeMJM_zX9QMq45KrkiC1M_1J22aQpmWtJTZlqtP3NY/edit  
* JOIN отправляется всем пользователям канала - что именно им отправляется?  
  + :dan-!d@localhost JOIN #test    ; dan- is joining the channel #test
* KICK отправляется всем пользователям канала - что именно им отправляется?
* QUIT отправляется всем пользователям канала - что именно им отправляется?
  + :dan-!d@localhost QUIT :Quit: Bye for now!
  + ; dan- is exiting the network with the message: "Quit: Bye for now!"

## проблемы второй срочности
* Stop a client (^-Z) connected on a channel. Then flood the channel using another client. When the client is live again, all stored commands should be processed normally. Check for memory leaks. (**checklist**)
  + **у меня не получается это протестировать, клиент после ^Z просто пропадает, как мне его сделать _live again_ ?**
* low bandwidth (checklist) - **я не поняла, как протестировать**
* OPER регистрация клиентом самого себя в качестве оператора канала
* MODE +ti
* Clients connecting from a host which name is longer than 63 characters are registered using the host (numeric) address instead of the host name
* Если сразу две проблемы, он какое из сообщений об ошибке отправляется клиенту?
* PRIVMSG: verify that is **fully functional with different parameters** (checklist)
  + **есть ли всё ещё в чеклисте это фраза?**
  + RCF2812 : The <target> parameter may also be a host mask (#<mask>)
  + The server will only send the PRIVMSG to those who have a host matching the mask
  + The mask MUST have at least 1 (one) "." in it and no wildcards following the last "."
  + Wildcards are the '*' and '?'  characters. It is only available to operators.
* 01 :Welcome to the Internet Relay Network molodoTR!Molod1@127.0.0.1 У в нашей версии почему-то обрубок текста
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
