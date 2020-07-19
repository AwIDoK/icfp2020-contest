#pragma once

#include "alien_data.h"
#include <cstdint>

struct StaticGameInfo {
    uint32_t maxTicks;
    bool role; // 0 - attacker, 1 - defender
    StaticGameInfo(const AlienData& data) {
        auto vector = data.getVector();
        maxTicks = vector[0].getNumber();
        role = vector[1].getNumber();
    }

};