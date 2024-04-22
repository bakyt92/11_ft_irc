#include "../headers/Server.hpp"

 std::string Server::uncapitalizeString(const std::string &input) {
	std::string output(input);
	for (size_t i = 0; i < input.size(); i++) {
		output[i] = tolower(input[i]);
	}
	return output;
}

std::queue<std::string> Server::split(const std::string &src, char delimiter, bool unique) {
	std::set<std::string> uniqueTokens;
	std::queue<std::string> tokensQueue;
	std::istringstream srcStream(src);
	std::string token;

	while (std::getline(srcStream, token, delimiter)) {
		if (unique) {
			uniqueTokens.insert(token);
		} else {
			tokensQueue.push(token);
		}
	}
	if (unique) {
		for (std::set<std::string>::const_iterator it = uniqueTokens.begin(); it != uniqueTokens.end(); ++it) {
			tokensQueue.push(*it);
		}
	}
	return tokensQueue;
}

bool Server::isValidName(const std::string &name) {
	if (name.empty()) {
		return false;
	}
	for (size_t i = 0; i < name.size(); i++) {
		if (name[i] == ' ' || name[i] == 0 || name[i] == 7 || name[i] == 13 || name[i] == 10) {
			return false;
		}
	}
	return true;
}

std::string Server::mergeTokensToString(const std::vector<std::string> &tokens, bool removeColon) {
	std::string mergedString;
	for (size_t i = 0; i < tokens.size(); ++i) {
		mergedString += tokens[i];
		if (i < tokens.size() - 1) {
			mergedString += " ";
		}
	}
	if (removeColon && !mergedString.empty() && mergedString[0] == ':') {
		mergedString.erase(0, 1);
	}
	return mergedString;
}

bool    Server::isNum(const std::string& str) {
    for (size_t i = 0; i < str.size(); ++i) {
        if (!isdigit(str[i]))
            return false;
    }
    return true;
}

bool    Server::isBitMask(const std::string& str) {
    if (!isNum(str))
        return false;
    int num;
    std::stringstream(str) >> num;

    // Check if only bits 2 or 3 are set
    return (num & (~0b1100)) == 0 && (num & 0b1100) != 0;
}

Mode    Server::getBitMode(const std::string str) {
    int num;
    std::istringstream(str) >> num;

    if (num & (1 << 3))
        return INVISIBLE;
    else
        return UNKNOWN;
}