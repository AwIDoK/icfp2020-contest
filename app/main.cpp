#include <iostream>
#include <regex>
#include <string>
#include "httplib.h"
#include "alien_data.h"
#include "game_response.h"
#include "comms.h"
#include "simulation.h"

using move_t = std::pair<int, int>;
using pos_t = std::pair<int, int>;

std::pair<int, int> oldEnemyPosition{};
int samePositionCount = 0;

ShipInfo getEnemyShip(bool role, GameResponse const& gameResponse) {
    for (auto const& ship : gameResponse.gameState.ships) {
        if (ship.isDefender != role && ship.params.divisionFactor > 0) {
            return ship;
        }
    }
    return gameResponse.gameState.ships[0]; // should never happen
}

bool isClose(std::pair<int, int> a, std::pair<int, int> b, int maxD) {
    return std::abs(a.first - b.first) <= maxD && std::abs(a.second - b.second) <= maxD;
}

std::pair<int, int> bestNavigatingMove(ShipInfo me, ShipInfo enemy, StaticGameInfo gameInfo, bool minimize=true) {
    std::vector<std::pair<int, int>> moves = {
            {1, 1},
            {1, 0},
            {1, -1},
            {0, 1},
            {0, 0},
            {0, -1},
            {-1, 1},
            {-1, 0},
            {-1, -1},
    };

    int64_t bestDistance = 1e16;
    if (!minimize) {
        bestDistance = 0;
    }
    std::pair<int, int> bestMove{0, 0};
    pos_t atPos{0, 0};

    for (int i = 0; i < moves.size(); i++) {
        for (int j = 0; j < moves.size(); j++) {
            for (int k = 0; k < moves.size(); k++) {
                auto nMe1 = predictShipState(me, moves[i]);
                auto nMe2 = predictShipState(nMe1, moves[j]);
                auto nMe3 = predictShipState(nMe2, moves[k]);
                auto nEnemy1 = samePositionCount >= 2 ? enemy : predictShipState(enemy, {0, 0});
                auto nEnemy2 = samePositionCount >= 2 ? nEnemy1 : predictShipState(nEnemy1, {0, 0});
                auto nEnemy3 = samePositionCount >= 2 ? nEnemy2 : predictShipState(nEnemy2, {0, 0});

                if (isBadPosition(nMe1, gameInfo) || isBadPosition(nMe2, gameInfo) || isBadPosition(nMe3, gameInfo)) {
                    continue;
                }
                auto myTrajectory = calculateTrajectory(nMe3);
                auto enemyTrajectory = calculateTrajectory(nEnemy3);
                bool goodTrajectory = true;
                for (int l = 0; l < std::min(size_t(6), myTrajectory.size()); l++) {
                    auto myPos = myTrajectory[l];
                    if (isBadPosition(myPos, gameInfo)) {
                        goodTrajectory = false;
                        break;
                    }
                }
                if (goodTrajectory) {
                    for (int l = 0; l < myTrajectory.size(); l++) {
                        auto myPos = myTrajectory[l];
                        auto enemyPos = samePositionCount >= 2 ? std::make_pair(enemy.x, enemy.y) : enemyTrajectory[l];
                        auto distance = getDistance2(myPos, enemyPos);
                        if ((minimize && distance < bestDistance) || (!minimize && distance > bestDistance)) {
                            bestDistance = distance;
                            bestMove = moves[i];
                            atPos = myPos;
                        }
                    }
                }
            }
        }
    }

    std::cout << "chose " << bestMove.first << " " << bestMove.second
                << " with dist " << bestDistance
                << " at pos " << atPos.first << " " << atPos.second << "\n";
    if (minimize && bestDistance == int64_t(1e16)) {
        return {100500, 100500};
    }
    if (!minimize && bestDistance == 0) {
        return {100500, 100500};
    }
    return bestMove;
}

int countAliveEnemies(GameResponse const& gameResponse) {
    int count = 0;
    for (auto const& ship : gameResponse.gameState.ships) {
        if (ship.isDefender != gameResponse.gameInfo.isDefender && ship.params.divisionFactor > 0) {
            count++;
        }
    }
    return count;
}

std::vector<AlienData> runStrategy(const GameResponse& gameResponse) {
    std::vector<AlienData> commands;
    bool role = gameResponse.gameInfo.isDefender;

    ShipInfo enemyShip = getEnemyShip(role, gameResponse);
    auto enemyPrediction = predictNextPosition(enemyShip);
    std::pair<int, int> enemyPosition{enemyShip.x, enemyShip.y};
    int enemiesAlive = countAliveEnemies(gameResponse);

    if (enemyPosition == oldEnemyPosition) {
        samePositionCount++;
    } else {
        oldEnemyPosition = enemyPosition;
        samePositionCount = 0;
    }

    for (auto const& ship : gameResponse.gameState.ships) {
        if (ship.isDefender != role) {
            // skipping ship
            continue;
        }

        int shipid = ship.id;
        std::pair<int, int> pos{ship.x, ship.y};
        std::pair<int, int> speed{ship.speed_x, ship.speed_y};

        if (ship.isDefender) {
            
            // hide from enemy
            auto move = bestNavigatingMove(ship, enemyShip, gameResponse.gameInfo, false);
            if (move.first == 100500) {
                bool safeOrbit = true;
                auto currentOrbit = calculateTrajectory(pos, speed);
                for (auto p: currentOrbit) {
                    if (isInsidePlanet(p, gameResponse.gameInfo)) {
                        safeOrbit = false;
                        break;
                    }
                }

                if (!safeOrbit) {
                    auto gravity = get_gravity(pos);
                    std::pair<int, int> move{signum(gravity.first + gravity.second), signum(gravity.second - gravity.first)};
                    commands.push_back(makeMoveCommand(shipid, move)); // gravity + gravity turned 90 degrees
                    continue;
                }
            } else {
                if (move.first != 0 || move.second != 0) {
                    commands.push_back(makeMoveCommand(shipid, move));
                    continue;
                }
            }
        } else {
            // follow enemy as attacker
            auto move = bestNavigatingMove(ship, enemyShip, gameResponse.gameInfo);
            if (move.first == 100500) {
                bool safeOrbit = true;
                auto currentOrbit = calculateTrajectory(pos, speed);
                for (auto p: currentOrbit) {
                    if (isInsidePlanet(p, gameResponse.gameInfo)) {
                        safeOrbit = false;
                        break;
                    }
                }

                if (!safeOrbit) {
                    auto gravity = get_gravity(pos);
                    std::pair<int, int> move{signum(gravity.first + gravity.second), signum(gravity.second - gravity.first)};
                    commands.push_back(makeMoveCommand(shipid, move)); // gravity + gravity turned 90 degrees
                    continue;
                }
            } else {
                auto nextPos = predict_movement(pos, speed, move).first;
                if (enemiesAlive == 1 && isClose(nextPos, enemyPrediction, 1)) {
                    commands.push_back(makeDestructCommand(shipid));
                }
                if (move.first != 0 || move.second != 0) {
                    commands.push_back(makeMoveCommand(shipid, move));
                    continue;
                }
            }
        }

        auto nextPos = predict_movement(pos, speed, {0, 0}).first;
        if (!role && isClose(nextPos, enemyPrediction, 1)) {
            commands.push_back(makeDestructCommand(shipid));
            continue;
        }

        // try to shoot
        int attack_power = std::max(0, std::min(ship.params.max_attack_power, ship.energy_limit - ship.current_energy));
        if (attack_power > 60) {
            if (samePositionCount >= 2 && enemyShip.params.power > 0) { // shoot at static target
                commands.push_back(makeShootCommand(shipid,
                                                    enemyPosition,
                                                    attack_power));
            } else { // shoot at moving target predicted position
                commands.push_back(makeShootCommand(shipid,
                                            enemyPrediction,
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

