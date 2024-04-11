План - 

/w

1. Найти ИРК рабочий и попробовать запустить https://github.com/AhmedFatir/ft_irc
2. Используем клиент  https://kiwiirc.com/nextclient/
3. Настя и вся команда - разберет серверную часть, все прочитаем гайд на 100 стр. (см. чат)
4. Найти видео - разобраться в концепции

#### Неоходимые функции IRC:
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

## Три основные части проекта:
1. Создайте все необходимые классы и методы для проекта.\n
2. Создайте сокет и обработайте сигналы сервера.
3. Используйте poll()функцию, чтобы проверить, произошло ли событие.
4. Если событие представляет собой новый клиент, зарегистрируйте его.
5. Если событие представляет собой данные зарегистрированного клиента, обработайте его.
6. Обработка различных команд IRC, таких как PASS, NICK, USER, JOIN, PART, TOPIC, INVITE, KICK, QUIT, MODE и PRIVMSG

Сокет - это конечная точка обмена данными между двумя системами. Это ключевой компонент для обмена данными между разными системами с использованием протоколов TCP и UDP. Сокеты используются в программировании для обеспечения коммуникации между сервером и клиентом.

## Протестировать
* `nick   an   `
* `nick '`

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
