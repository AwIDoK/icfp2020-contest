#pragma once

#include <cstdint>

struct ShipInfo {
    uint32_t gameTick;
    bool role; // 0 - attacker, 1 - defende;
};