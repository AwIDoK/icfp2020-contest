#pragma once

#include <algorithm>
#include <cassert>
#include <string>
#include <vector>
#include "alien_data.h"

int64_t fromBinary(const std::string& string, size_t start, size_t end) {
    int64_t result = 0;
    for (size_t i = 0; i < end; i++) {
        result = result * 2 + (string[i] == '1' ? 1 : 0);
    }
    return result;
}

std::string toBinary(int64_t num) {
    std::string result;
    if (num == 0) {
        return "0";
    }
    while (num) {
        result += num & 1 ? '1' : '0';
        num <<= 1;
    }
    std::reverse(result.begin(), result.end());
    return result;
}


AlienData decodeAlien(const std::string& encoded, size_t currentPosition, size_t& returnPosition) {
    auto reminder = currentPosition + 2;
    if (encoded[currentPosition] == '0' && encoded[currentPosition + 1] == '0') {
        returnPosition = reminder;
        return std::vector<AlienData>();
    } else if (encoded[currentPosition] == '1' && encoded[currentPosition + 1] == '1') {
        size_t cont, cont2;
        auto a = decodeAlien(encoded, reminder, cont);
        auto b = decodeAlien(encoded, cont, cont2);
        returnPosition = cont2;
        if (b.isVector()) {
            auto& vector = b.getVector();
            vector.insert(vector.begin(), a);
            return vector;
        } else {
            return VectorPair<AlienData>(a, b);
        }
    } else {
        auto pos = reminder;
        while(encoded[pos] != '0') {
            pos++;
        }
        pos -= reminder;
        if (pos == 0) {
            returnPosition = reminder + 1;
            return 0;
        }
        auto end = pos + 1 + 4 * pos;
        returnPosition = reminder + end;
        auto sign = encoded[currentPosition] == '1' ? -1 : 1;
        return sign * fromBinary(encoded, reminder + pos + 1, reminder + end);
    }
}

AlienData decodeAlien(const std::string& data) {
    size_t cont = 0;
    auto decoded = decodeAlien(data, 0, cont);
    if (cont != data.size()) {
        std::cout << "failed decoding " << data << std::endl;
        std::cout << cont << ' ' << data.size() << std::endl;
    }
    assert(cont == data.size());
    return decoded;
}

std::string encodeInt(int64_t number) {
    if (number == 0) {
        return "010";
    }
    std::string prefix = number > 0 ? "01" : "10";
    auto numberStr = toBinary(abs(number));
    auto notPaddingSize = numberStr.size() % 4;
    auto paddingSize = notPaddingSize == 0 ? 0 : 4 - notPaddingSize;
    auto paddedNumberStr = std::string(paddingSize, '0') + numberStr;
    auto length = paddedNumberStr.size() / 4;
    return prefix + std::string(length, '1') + '0' + paddedNumberStr;
}

std::string encodeAlien(const AlienData& data) {
    if (data.isVector()) {
        const auto& vector = data.getVector();
        std::string result;
        for (const auto& element : vector) {
            result += "11";
            result += encodeAlien(element);
        }
        result += "00";
        return result;
    } else if (data.isPair()) {
        const auto& pair = data.getPair();
        return "11" + encodeAlien(pair.first()) + encodeAlien(pair.second());
    } else {
        return encodeInt(data.getNumber());
    }
}
