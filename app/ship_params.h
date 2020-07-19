#pragma once

#include "alien_data.h"
#include <cstdint>


struct ShipParams {
    int32_t power;
    int32_t max_attack_power;
    int32_t generation;
    int32_t divisionFactor;

    ShipParams(const AlienData& data) {
        const auto& vector = data.getVector();
        power = vector[0].getNumber();
        max_attack_power = vector[1].getNumber();
        generation = vector[2].getNumber();
        divisionFactor = vector[3].getNumber();
    }

    ShipParams() {}
};