#pragma once

#include "alien_data.h"
#include <cstdint>

struct StaticGameInfo {
    uint32_t maxTicks;
    bool isDefender; // 0 - attacker, 1 - defender
    int32_t planetSize;
    int32_t worldSize;

    explicit StaticGameInfo(const AlienData& data) {
        auto vector = data.getVector();
        maxTicks = vector[0].getNumber();
        isDefender = vector[1].getNumber();
        planetSize = vector[3].getVector()[0].getNumber();
        worldSize = vector[3].getVector()[1].getNumber();
    }

    StaticGameInfo() {
    }

};