#pragma once

#include "alien_data.h"
#include <cstdint>

struct StaticGameInfo {
    uint32_t maxTicks;
    bool role; // 0 - attacker, 1 - defender
    int32_t planetSize;

    explicit StaticGameInfo(const AlienData& data) {
        auto vector = data.getVector();
        maxTicks = vector[0].getNumber();
        role = vector[1].getNumber();
        planetSize = vector[3].getVector()[0].getNumber();
    }

    StaticGameInfo() {
    }

};