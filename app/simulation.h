#pragma once

#include <utility>
#include <cmath>
#include "ship_info.h"
#include "static_game_info.h"

int signum(int x) {
    if (x == 0) {
        return 0;
    }
    return x > 0 ? 1 : -1;
}

int64_t sqr(int32_t x) {
    return static_cast<int64_t>(x) * x;
}

int64_t getDistance2(std::pair<int, int> a, std::pair<int, int> b) {
    return sqr(a.first - b.first) + sqr(a.second - b.second);
}

int64_t getDistance2(ShipInfo const& a, ShipInfo const& b) {
    return getDistance2({a.x, a.y}, {b.x, b.y});
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

std::pair<int, int> predictNextPosition(ShipInfo const& info, std::pair<int, int> move) {
    return predict_movement({info.x, info.y}, {info.speed_x, info.speed_y}, {0, 0}).first;
}

std::pair<int, int> predictNextPosition(ShipInfo const& info) {
    return predictNextPosition(info, {0, 0});
}

std::vector<std::pair<int, int>> calculateTrajectory(std::pair<int, int> position, std::pair<int, int> speed) {
    constexpr int LOOKAHEAD = 100;
    std::vector<std::pair<int, int>> result(LOOKAHEAD);

    for (int i = 0; i < LOOKAHEAD; i++) {
        auto tmp = predict_movement(position, speed, {0, 0});
        position = tmp.first;
        speed = tmp.second;
        result[i] = position;
    }

    return result;
}

std::vector<std::pair<int, int>> calculateTrajectory(ShipInfo const& ship) {
    return calculateTrajectory({ship.x, ship.y}, {ship.speed_x, ship.speed_y});
}

// todo: heat predictions
ShipInfo predictShipState(ShipInfo ship, std::pair<int, int> move) {
    if (ship.params.power == 0) {
        return ship;
    }

    auto movement = predict_movement({ship.x, ship.y}, {ship.speed_x, ship.speed_y}, move);

    ship.x = movement.first.first;
    ship.y = movement.first.second;

    ship.speed_x = movement.second.first;
    ship.speed_y = movement.second.second;

    ship.params.power--;

    return ship;
}

bool isInsidePlanet(std::pair<int, int> pos, StaticGameInfo const& gameInfo) {
    return std::abs(pos.first) <= gameInfo.planetSize && std::abs(pos.second) <= gameInfo.planetSize;
}

bool isInsideWorld(std::pair<int, int> pos, StaticGameInfo const& gameInfo) {
    return std::abs(pos.first) < gameInfo.worldSize && std::abs(pos.second) < gameInfo.worldSize;
}

bool isBadPosition(std::pair<int, int> pos, StaticGameInfo const& gameInfo) {
    return isInsidePlanet(pos, gameInfo) || !isInsideWorld(pos, gameInfo);
}

bool isBadPosition(ShipInfo const& ship, StaticGameInfo const& gameInfo) {
    return isInsidePlanet({ship.x, ship.y}, gameInfo) || !isInsideWorld({ship.x, ship.y}, gameInfo);
}
