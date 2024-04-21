## Разобраться или доделать
* одна команда может оказаться разбитой на несколько сообщений или нет ?
  + Кажется у Бориса не может
  + TCP is a streaming protocol, not a message protocol
    - The only guarantee is that you send n bytes, you will receive n bytes in the same order.
    - You might send 1 chunk of 100 bytes and receive 100 1 byte recvs, or you might receive 20 5 bytes recvs.
    - You could send 100 1 byte chunks and receive 4 25 byte messages.
    - **You must deal with message boundaries yourself**. (у нас messages boundaries это `\n`, правильно?)
  + Из RFC 1459: В предоставление полезной 'non-buffered' сети IO для клиентов и серверов, каждое соединение из которых является частным 'input buffer', в котором результируются большинство полученного, читается и проверяется. Размер буфера 512 байт, используется как одно полное сообщение, хотя обычно оно бывает с разными командам. Приватный буфер проверяется после каждой операции чтения на правильность сообщений. Когда распределение с многослойными сообщениями от одного клиента в буфере, следует быть в качестве одного случившегося, клиент может быть 'удален'.
  + `com^Dman^Dd` (* use ctrl+D **to send the command in several parts**: `com`, then `man`, then `d\n`) - что это значит?
* должна ли PRIVMSG понимать маски и особые формы записи?
  + `PRIVMSG #*.edu :NSFNet is undergoing work, expect interruptions` Сообщение для всех пользователей, сидящих на хосте, попадающим под маску *.edu
  + Борис проверяет `"${receiver}"`зачем-то
  + в дискорде пишут надо только `#` https://discord.com/channels/774300457157918772/785407578972225579/922447406606458890
  + Параметр <receiver> может быть маской хоста (#mask) или маски сервера ($mask)
    - Cервер будет отсылать PRIVMSG только тем, кто попадает под серверную или хост-маску
    - Маска должна содержать в себе как минимум одну "." - это требование вынуждаеит пользователей отсылать сообщения к "#*" или "$*", которые уже потом рассылаются всем пользователям; по опыту, этим злоупотребляет большое количество пользователей
    - В масках используются '*' и '?', это расширение команды PRIVMSG доступно только IRC-операторам
* Имя канала обязательно начинается с `#`? Я пока сделала так, но кажется это неправильно, вроде бы `#` это маска хоста
* `JOIN #foo,&bar fubar` вход на канал #foo, используя ключ "fubar" и на канал &bar без использования ключа - я пока ничего не сделала насчёт `&`, надо ли?
* я сделала `MODE` для установки одного параметра за раз, например `MODE -t` должна работать, а `MODE -tpk` нет, нормально ли это? 
* Просмотреть группу в дискорд
* Посмотреть другие проекты
* сверить наш код с `bircd` (который нам дали как пример с сабжектом) - у меня не получается, этот код слишком отличается от нашего 
* `valgrind` (в конце)
                               
## Протестировать нашу программу и реальный сервер
* [rfc1459](https://github.com/bakyt92/11_ft_irc/blob/master/docs/rfc1459.txt) (цитата: RFC 1459 is famously sparse. It does not tell you everything you need to know to write a server)
* [rfc2812](https://datatracker.ietf.org/doc/html/rfc2812)
* наша упрощённая версия НЕ во всём должна работать как настощий сервер (вроде нам не нужны маски, у нас только один сервер, ...)
* Как выглядит параметр `mode`, когда сервер его показывает ?
  + https://stackoverflow.com/questions/12886573/implementing-irc-rfc-how-to-respond-to-mode
* `MODE #myChannel -lktt`
* `MODE #myChannel +ltk` 5 myTopic myPass
* неправильное имя канала
* неправильнй пароль (по RCF 1459 и RCF 2812 не понятно!) 
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
* https://github.com/opsec-infosec/ft_irc-tester
* https://github.com/markveligod/ft_irc

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
* [Аня разбирает код тут](https://github.com/akostrik/IRC-fork/blob/master/README.md)  

