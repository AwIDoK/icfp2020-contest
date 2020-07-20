#pragma once

#include "alien_data.h"
#include "ship_params.h"
#include <cstdint>

struct ShipInfo {
    bool isDefender; // 0 - attacker, 1 - defender
    int32_t id;
    int32_t x, y;
    int32_t speed_x, speed_y;
    ShipParams params;
    int32_t current_energy;
    int32_t energy_limit;
    int32_t unknown;

    ShipInfo(const AlienData& data) {
        const auto& vector = data.getVector();

        const auto& ship = vector[0].getVector();

        isDefender = ship[0].getNumber();
        id = ship[1].getNumber();

        x = ship[2].getPair().first().getNumber();
        y = ship[2].getPair().second().getNumber();
        speed_x = ship[3].getPair().first().getNumber();
        speed_y = ship[3].getPair().second().getNumber();

        params = ShipParams(ship[4]);
        current_energy = ship[5].getNumber();
        energy_limit = ship[6].getNumber();
        unknown = ship[7].getNumber();
        const auto& commands = data.getVector()[1].getVector();
    }
};