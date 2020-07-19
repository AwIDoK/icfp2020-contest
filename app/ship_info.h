#pragma once

#include "alien_data.h"
#include "ship_params.h"
#include <cstdint>

struct ShipInfo {
    bool role; // 0 - attacker, 1 - defender
    int32_t id;
    int32_t x, y;
    int32_t speed_x, speed_y;
    ShipParams params;
    int32_t current_energy;
    int32_t energy_limit;
    int32_t unknown;

    ShipInfo(const AlienData& data) {
        const auto& ship = data.getVector()[0].getVector();
        role = ship[0].getNumber();
        id = ship[1].getNumber();
        x = ship[2].getPair().first().getNumber();
        y = ship[2].getPair().second().getNumber();

        speed_x = ship[2].getPair().first().getNumber();
        speed_y = ship[2].getPair().second().getNumber();

        params = ShipParams(ship[3]);
        current_energy = ship[4].getNumber();
        energy_limit = ship[5].getNumber();
        unknown = ship[6].getNumber();
        const auto& commands = data.getVector()[1].getVector();
    }
};