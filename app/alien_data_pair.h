#pragma once

#include <vector>

struct AlienDataPair {
    std::vector<AlienData> data;
    AlienDataPair(const AlienData& a, const AlienData& b) : data({a, b}) {}
    AlienData& first() {
        return data[0];
    }

    AlienData& second() {
        return data[1];
    }

    AlienData first() const {
        return data[0];
    }

    AlienData second() const {
        return data[1];
    }
};
