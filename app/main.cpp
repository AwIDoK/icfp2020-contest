#include <iostream>
#include <regex>
#include <string>
#include "httplib.h"
#include "alien_data.h"
#include "game_response.h"
#include "comms.h"

std::pair<int, int> oldEnemyPosition{};
int samePositionCount = 0;

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

std::pair<int, int> predictNextPosition(ShipInfo const& info) {
    return predict_movement({info.x, info.y}, {info.speed_x, info.speed_y}, {0, 0}).first;
}

std::vector<std::pair<int, int>> calculateOrbit(std::pair<int, int> position, std::pair<int, int> speed) {
    constexpr int LOOKAHEAD = 50;
    std::vector<std::pair<int, int>> result(LOOKAHEAD);

    for (int i = 0; i < LOOKAHEAD; i++) {
        auto tmp = predict_movement(position, speed, {0, 0});
        position = tmp.first;
        speed = tmp.second;
        result[i] = position;
    }

    return result;
}

ShipInfo getEnemyShip(bool role, GameResponse const& gameResponse) {
    for (auto const& ship : gameResponse.gameState.ships) {
        if (ship.role != role && ship.params.divisionFactor > 0) {
            return ship;
        }
    }
    return gameResponse.gameState.ships[0]; // should never happen
}

bool isClose(std::pair<int, int> a, std::pair<int, int> b, int maxD) {
    return std::abs(a.first - b.first) <= maxD && std::abs(a.second - b.second) <= maxD;
}

std::vector<AlienData> runStrategy(const GameResponse& gameResponse) {
    std::vector<AlienData> commands;
    int planetSize = gameResponse.gameInfo.planetSize;
    bool role = gameResponse.gameInfo.role;

    ShipInfo enemyShip = getEnemyShip(role, gameResponse);
    auto enemyPrediction = predictNextPosition(enemyShip);
    std::pair<int, int> enemyPosition{enemyShip.x, enemyShip.y};

    if (enemyPosition == oldEnemyPosition) {
        samePositionCount++;
    } else {
        oldEnemyPosition = enemyPosition;
        samePositionCount = 0;
    }


    for (auto const& ship : gameResponse.gameState.ships) {
        if (ship.role != role) {
            // skipping ship
            continue;
        }

        int shipid = ship.id;
        std::pair<int, int> pos{ship.x, ship.y};
        std::pair<int, int> speed{ship.speed_x, ship.speed_y};

        bool safeOrbit = true;
        auto currentOrbit = calculateOrbit(pos, speed);
        for (auto p: currentOrbit) {
            if (std::abs(p.first) <= planetSize && std::abs(p.second) <= planetSize) {
                safeOrbit = false;
                break;
            }
        }

        if (!safeOrbit) {
            auto gravity = get_gravity(pos);
            std::pair<int, int> move{gravity.first + gravity.second, gravity.second - gravity.first};
            commands.push_back(makeMoveCommand(shipid, VectorPair<AlienData>(move.first, move.second))); // gravity + gravity turned 90 degrees
            auto nextPos = predict_movement(pos, speed, move).first;
            if (!role && isClose(nextPos, enemyPrediction, 1)) {
                commands.push_back(makeDestructCommand(shipid));
            }
            continue;
        }

        auto nextPos = predict_movement(pos, speed, {0, 0}).first;
        if (!role && isClose(nextPos, enemyPrediction, 1)) {
            commands.push_back(makeDestructCommand(shipid));
            continue;
        }

        // try to shoot
        int attack_power = std::max(0, std::min(ship.params.max_attack_power, ship.energy_limit - ship.current_energy));
        if (attack_power > 60) {
            if (samePositionCount >= 2 && enemyShip.params.power > 0) {
                commands.push_back(makeShootCommand(shipid,
                                                    VectorPair<AlienData>(enemyPosition.first, enemyPosition.second),
                                                    attack_power));
            } else {
                commands.push_back(makeShootCommand(shipid,
                                            VectorPair<AlienData>(enemyPrediction.first, enemyPrediction.second),
                                                    attack_power));
            }
            continue;
        }
    }

    return commands;
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
    auto response = send(client, serverUrl, joinRequest);
    std::cout << response.to_string() << std::endl;
    auto startRequest = makeStartRequest(playerKey, response);
    response = send(client, serverUrl, startRequest);

    while (response.getVector()[1].getNumber() != 2) {
    	std::cout << response.to_string() << std::endl;
    	GameResponse gameResponse{response};
    	auto commands = runStrategy(gameResponse);
        auto commandsRequest = makeCommandsRequest(playerKey, commands);
        response = send(client, serverUrl, commandsRequest);
	}
    
    std::cout << response.to_string() << std::endl;
	
	return 0;
}

