#pragma once

#include "alien_data.h"
#include <cstdint>

struct ShipInfo {
    int32_t x, y;
    int32_t speed_x, speed_y;

    ShipInfo(const AlienData& data) {
        
    }
};