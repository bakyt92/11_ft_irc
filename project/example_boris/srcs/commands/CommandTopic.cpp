#include "CommandTopic.hpp"

CommandTopic::CommandTopic() : ACommand() {}
CommandTopic::CommandTopic(Kernel *kernel) : ACommand(kernel){}
CommandTopic::~CommandTopic() {}

std::string CommandTopic::exec(){
	showMe();
	int result = 0;
	Channel	*currChannel;
	std::string topic;
	std::string answer;
	std::string servicePart;

	if (_args.size() < 2)
		result = ERR_NEEDMOREPARAMS;
	else
	{
		currChannel = _kernel->_channels->getChannelByName(_args[1]);
		if (!currChannel)
			result = ERR_NOSUCHCHANNEL;
		else if (!currChannel->isContainList(currChannel->getListOfClient(), _client))
			result = ERR_NOTONCHANNEL;
		else if (_args.size() == 2)
		{
			topic = currChannel->getTopic();
			answer = currChannel->getName() + " :" + \
					((topic.empty()) ? "No topic is set" : topic);
			int rpl = (topic.empty()) ? RPL_NOTOPIC : RPL_TOPIC;
			return Response::getInstance().getResponse(rpl, "",
				_client->getNickName(), answer);
		}
		else
		{
			if (currChannel->setTopic(_args[2], _client))
			{
				topic = currChannel->getTopic();
				answer = currChannel->getName() + " :" + \
						((topic.empty()) ? "No topic is set" : topic);
				currChannel->sendMessageToChannel(RPL_TOPIC, answer);
			}
			else
				return Response::getInstance().getResponse(
					ERR_CHANOPRIVSNEEDED, "", _client->getNickName(),
					_args[0]);
		}
	}
	return (result) ? Response::getInstance().getResponse
		(result, "", _client->getNickName(), _args[0]) : "";
}
