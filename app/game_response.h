#pragma once

#include <cstdint>
#include "alien_data.h"
#include <cassert>
#include "iostream"
#include "static_game_info.h"
#include "game_state.h"

struct GameResponse {
    uint32_t responseStatus;
    uint32_t gameStatus;
    StaticGameInfo gameInfo;
    GameState gameState;

    explicit GameResponse(const AlienData& data) {
        auto vector = data.getVector();
        if (vector.size() == 1) {
            std::cout << "Bad request";
            assert(false);
        }
        responseStatus = vector[0].getNumber();
        gameStatus = vector[1].getNumber();
        gameInfo = StaticGameInfo(vector[2]);
        gameState = GameState(vector[3]);
    }
};