#pragma once

#include <cstdint>
#include "ship_info.h"

struct GameState {
    uint32_t gameTick;

    std::vector<ShipInfo> ships;

    GameState(const AlienData& data) {
        auto vector = data.getVector();
        gameTick = vector[0].getNumber();
        for (auto const& ship : vector[2].getVector()) {
            ships.emplace_back(ship);
        }
    }
};