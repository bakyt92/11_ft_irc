#include "Response.hpp"
#include "Kernel.hpp"

Response::Response() {
    _reply[200] = "Link <version & debug level> <destination> <next server>";
    _reply[201] = "Try. <class> <server>";
    _reply[202] = "H.S. <class> <server>";
    _reply[203] = "???? <class> [<client IP address in dot form>]";
    _reply[204] = "Oper <class> <nick>";
    _reply[205] = "User <class> <nick>";
    _reply[206] = "Serv <class> <int>S <int>C <server> <nick!user|*!*>@<host|server>";
    _reply[208] = "<newtype> 0 <client name>";
    _reply[211] = "<linkname> <sendq> <sent messages> <sent bytes> <received messages> <received bytes> <time open>";
    _reply[212] = "<command> <count>";
    _reply[213] = "C <host> * <name> <port> <class>";
    _reply[214] = "N <host> * <name> <port> <class>";
    _reply[215] = "I <host> * <host> <port> <class>";
    _reply[216] = "K <host> * <username> <port> <class>";
    _reply[218] = "Y <class> <ping frequency> <connect frequency> <max sendq>";
    _reply[219] = "<stats letter> :End of /STATS report";
    _reply[221] = ""; //"<user mode string>";
    _reply[241] = "L <hostmask> * <servername> <maxdepth>";
    _reply[242] = ":Server Up %d days %d:%02d:%02d";
    _reply[243] = "O <hostmask> * <name>";
    _reply[244] = "H <hostmask> * <servername>";
    _reply[251] = ":There are <integer> users and <integer> invisible on <integer> servers";
    _reply[252] = "<integer> :operator(s) online";
    _reply[253] = "<integer> :unknown connection(s)";
    _reply[254] = "<integer> :channels formed";
    _reply[255] = ":I have <integer> clients and <integer> servers";
    _reply[256] = "<server> :Administrative info";
    _reply[257] = ":<admin info>";
    _reply[258] = ":<admin info>";
    _reply[259] = ":<admin info>";
    _reply[261] = "File <logfile> <debug level>";
    _reply[300] = "";
    _reply[301] = "<nick> :<away message>";
    _reply[302] = ""; //:[<reply>{<space><reply>}]";
    _reply[303] = ":[<nick> {<space><nick>}]";
    _reply[305] = ":You are no longer marked as being away";
    _reply[306] = ":You have been marked as being away";
    _reply[311] = "<nick> <user> <host> * :<real name>";
    _reply[312] = "<nick> <server> :<server info>";
    _reply[313] = "<nick> :is an IRC operator";
    _reply[314] = "<nick> <user> <host> * :<real name>";
    _reply[315] = ""; //<name> :End of /WHO list";
    _reply[317] = "<nick> <integer> :seconds idle";
    _reply[318] = "<nick> :End of /WHOIS list";
    _reply[319] = "<nick> :{[@|+]<channel><space>}";
    _reply[321] = "Channel :Users Name";
    _reply[322] = "";//<channel> <# visible> :<topic>";
    _reply[323] = ":End of /LIST";
    _reply[324] = "";//<channel> <mode> <mode params>";
    _reply[331] = "";//<channel> :No topic is set";
    _reply[332] = "";//<channel> :<topic>";
    _reply[341] = "<channel> <nick>";
    _reply[342] = "<user> :Summoning user to IRC";
    _reply[351] = "<version>.<debuglevel> <server> :<comments>";
    _reply[352] = "";//<channel> <user> <host> <server> <nick> <H|G>[*][@|+]:<hopcount> <real name>
    _reply[353] = "";// <channel> :[[@|+]<nick> [[@|+]<nick> [...]]]
    _reply[364] = "<mask> <server> :<hopcount> <server info>";
    _reply[365] = "<mask> :End of /LINKS list";
    _reply[366] = "";//<channel> :End of /NAMES list";
    _reply[367] = "";//<channel> <banid>";
    _reply[368] = "";//<channel> :End of channel ban list";
    _reply[369] = "<nick> :End of WHOWAS";
    _reply[371] = ":<string>";
    _reply[372] = ""; // ":- <text>"; Denis
    _reply[374] = ":End of /INFO list";
    _reply[375] = "";//:- ircServer Message of the day - "; //Denis
    _reply[376] = ""; //":End of /MOTD command";
    _reply[381] = ":You are now an IRC operator";
    _reply[382] = "<config file> :Rehashing";
    _reply[391] = "<server> :<string showing server's local time>";
    _reply[391] = "<server> :<string showing server's local time>";
    _reply[392] = ":UserID Terminal Host";
    _reply[393] = ":%-8s %-9s %-8s";
    _reply[394] = ":End of users";
    _reply[395] = ":Nobody logged in";
    _reply[401] = ":No such nick";
    _reply[402] = "<server name> :No such server";
    _reply[403] = ":No such channel";
    _reply[404] = ":Cannot send to channel";
    _reply[405] = "<channel name> :You have joined too many channels";
    _reply[406] = "<nickname> :There was no such nickname";
    _reply[407] = ":Duplicate recipients. No message delivered";
    _reply[409] = ":No origin specified";
    _reply[411] = ":No recipient given (<command>)";
    _reply[412] = ":No text to send";
    _reply[413] = ":No toplevel domain specified";
    _reply[414] = ":Wildcard in toplevel domain";
    _reply[421] = ":Unknown command";
    _reply[422] = ":MOTD File is missing";
    _reply[423] = "<server> :No administrative info available";
    _reply[424] = ":File error doing <file op> on <file>";
    _reply[431] = ":No nickname given";
    _reply[432] = ":Erroneus nickname";
    _reply[433] = "<nick> :Nickname is already in use";
    _reply[436] = "<nick> :Nickname collision KILL";
    _reply[441] = "<nick> <channel> :They aren't on that channel";
    _reply[442] = ":You're not on that channel";
    _reply[443] = "<user> <channel> :is already on channel";
    _reply[444] = "<user> :User not logged in";
    _reply[445] = ":SUMMON has been disabled";
    _reply[446] = ":USERS has been disabled";
    _reply[451] = ":You have not registered";
    _reply[461] = ":Not enough parameters";
    _reply[462] = ":You may not reregister";
    _reply[463] = ":Your host isn't among the privileged";
    _reply[464] = ":Password incorrect";
    _reply[465] = ":You are banned from this server";
    _reply[467] = ":Channel key already set";
    _reply[471] = ":Cannot join channel (+l)";
    _reply[472] = ":is unknown mode char to me"; //"<char> :is unknown mode char to me";
    _reply[473] = ":Cannot join channel (+i)";
    _reply[474] = ":Cannot join channel (+b)";
    _reply[475] = ":Cannot join channel (+k)";
    _reply[481] = ":Permission Denied- You're not an IRC operator";
    _reply[482] = ":You're not channel operator";
    _reply[483] = ":You cant kill a server!";
    _reply[491] = ":No O-lines for your host";
    _reply[501] = ":Unknown MODE flag";
    _reply[502] = ":Cant change mode for other users";
}
Response::Response(Response const &source) {*this = source;}
Response &Response::operator=(Response const &source) {
	if (this != &source) {}
	return *this;
}

void Response::setServerName(std::string name) {
    _serverName = name;
}

std::string	itoa(int n)
{
	std::string result;

	if (n < 10) return (result += n + 48);
	return (result += itoa(n / 10) + char(n % 10 + 48));
}

std::string Response::getResponse(int code, std::string from, std::string to, std::string args) {
    std::string out = ":";
    if (from == "")
        out += _serverName;
    else
        out += from;
    if (code)
        out += " " + itoa(code);
    if (to != "")
        out += " " + to;
    if (args != "")
        out += " " + args;
    if (code)
        out += " " + _reply[code] + '\n';
    return out;
}
