#pragma once

#include <regex>
#include <string>
#include "httplib.h"
#include "alien_data.h"
#include "translator.h"
#include "game_response.h"

AlienData convertToAlien(std::pair<int, int> p) {
    return VectorPair<AlienData>(p.first, p.second);
}

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
    auto unknownVec = std::vector<AlienData>({192496425430ll, 114342420ll});
    return std::vector<AlienData>({requestTypeData, playerKey, unknownVec});
}


AlienData makeStartRequest(int64_t playerKey, const AlienData& gameResponse) {
    auto requestTypeData = 3;
    std::vector<AlienData> shipParams;
    if (StaticGameInfo(gameResponse.getVector()[2]).isDefender) {
        shipParams = std::vector<AlienData>({326, 0, 10, 1});
    } else {
        shipParams = std::vector<AlienData>({134, 64, 10, 1});
    }
    return std::vector<AlienData>({requestTypeData, playerKey, shipParams});
}

AlienData makeMoveCommand(int64_t id, AlienData const& move) {
    return std::vector<AlienData>({0, id, move});
}

AlienData makeMoveCommand(int64_t id, std::pair<int, int> move) {
    return std::vector<AlienData>({0, id, convertToAlien(move)});
}

AlienData makeDestructCommand(int64_t id) {
    return std::vector<AlienData>({1, id});
}

AlienData makeShootCommand(int64_t id, AlienData const& position, int64_t power) {
    return std::vector<AlienData>({2, id, position, power});
}

AlienData makeShootCommand(int64_t id, std::pair<int, int> position, int64_t power) {
    return std::vector<AlienData>({2, id, convertToAlien(position), power});
}

AlienData makeCommandsRequest(int64_t playerKey, const std::vector<AlienData>& commandList) {
    auto requestTypeData = 4;
    return std::vector<AlienData>({requestTypeData, playerKey, commandList});
}
