#include <iostream>
#include <regex>
#include <string>
#include "httplib.h"
#include "alien_data.h"
#include "translator.h"


AlienData send(httplib::Client& client, const std::string& serverUrl, const AlienData& data) {
	auto encoded = encodeAlien(data);
	std::cout << "sending " << data.to_string() << ' ' << encoded << std::endl;
	const std::shared_ptr<httplib::Response> serverResponse = 
		client.Post((serverUrl + "/aliens/send").c_str(), encoded.c_str(), "text/plain");

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
	auto commandList = std::vector<AlienData>({command});
    return std::vector<AlienData>({requestTypeData, playerKey, commandList});
}

int signum(int x) {
    if (x == 0) {
        return 0;
    }
    return x > 0 ? 1 : -1;
}

std::pair<int, int> get_gravity(std::pair<int, int> position) {
    int x = position.first;
    int y = position.second;

    int gx = 0;
    int gy = 0;

    // separate ifs because if x == y gravity works in both directions
    // same for x == -y
    if (std::abs(x) >= std::abs(y)) {
        gx = -signum(x);
    }

    if (std::abs(x) <= std::abs(y)) {
        gy = -signum(y);
    }

    return {gx, gy};
}

// what position and speed we will get if now we have position and speed and send move command
std::pair<std::pair<int, int>, std::pair<int, int>> predict_movement(std::pair<int, int> position, std::pair<int, int> speed, std::pair<int, int> moveCommand) {
    auto gravity = get_gravity(position);

    std::pair newSpeed{speed.first + gravity.first - moveCommand.first,
                       speed.second + gravity.second - moveCommand.second};
    std::pair newPos{position.first + newSpeed.first, position.second + newSpeed.second};

    return {newPos, newSpeed};
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

