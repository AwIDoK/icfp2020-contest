#pragma once

#include <cstdint>

struct GameState {
    uint32_t gameTick;
    bool role; // 0 - attacker, 1 - defende;
};