#include <iostream>
#include <regex>
#include <string>
#include "httplib.h"
#include "alien_data.h"
#include "translator.h"


AlienData send(httplib::Client& client, const std::string& serverUrl, const AlienData& data) {
	auto encoded = encodeAlien(data);
	const std::shared_ptr<httplib::Response> serverResponse = 
		client.Post(serverUrl.c_str(), encoded.c_str(), "text/plain");

	if (!serverResponse) {
		std::cout << "Unexpected server response:\nNo response from server" << std::endl;
		return 1;
	}
	
	if (serverResponse->status != 200) {
		std::cout << "Unexpected server response:\nHTTP code: " << serverResponse->status
		          << "\nResponse body: " << serverResponse->body << std::endl;
		return 2;
	}

	std::cout << "Server response: " << serverResponse->body << std::endl;
    return decodeAlien(serverResponse->body);
}


AlienData makeJoinRequest(int64_t playerKey) {
	auto requestTypeData = 2;
	auto unknownVec = std::vector<AlienData>();
	return std::vector<AlienData>({requestTypeData, playerKey, unknownVec});
}


AlienData makeStartRequest(int64_t playerKey, const AlienData& gameResponse) {
	auto requestTypeData = 3;
	auto shipParams = std::vector<AlienData>({2, 3, 4, 5});
	return std::vector<AlienData>({requestTypeData, playerKey, shipParams});
}

AlienData makeCommandsRequest(int64_t playerKey, const AlienData& gameResponse) {
	auto requestTypeData = 4;
	auto command = std::vector<AlienData>({0, 0, VectorPair<AlienData>(-1, -1)});
    return std::vector<AlienData>({requestTypeData, playerKey, command});
}


int main(int argc, char* argv[]) {
	const std::string serverUrl(argv[1]);
	const int64_t playerKey(std::stoll(argv[2]));

	std::cout << "ServerUrl: " << serverUrl << "; PlayerKey: " << playerKey << std::endl;
	
	const std::regex urlRegexp("http://(.+):(\\d+)");
	std::smatch urlMatches;
	if (!std::regex_search(serverUrl, urlMatches, urlRegexp) || urlMatches.size() != 3) {
		std::cout << "Unexpected server response:\nBad server URL" << std::endl;
		return 1;
	}
	const std::string serverName = urlMatches[1];
	const int serverPort = std::stoi(urlMatches[2]);
	httplib::Client client(serverName, serverPort);

	auto joinRequest = makeJoinRequest(playerKey);
    auto gameResponse = send(client, serverUrl, joinRequest);
    std::cout << gameResponse.to_string() << std::endl;
    auto startRequest = makeStartRequest(playerKey, gameResponse);
    gameResponse = send(client, serverUrl, startRequest);

    while (gameResponse.getVector()[1].getNumber() != 2) {
    	std::cout << gameResponse.to_string() << std::endl;
        auto commandsRequest = makeCommandsRequest(playerKey, gameResponse);
        gameResponse = send(client, serverUrl, commandsRequest);
	}
    
    std::cout << gameResponse.to_string() << std::endl;
	
	return 0;
}

