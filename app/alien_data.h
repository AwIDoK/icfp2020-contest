#pragma once

#include <variant>
#include <vector>

template <typename T>
struct VectorPair {
    std::vector<T> data;
    VectorPair(const T& a, const T& b) : data({a, b}) {}
    T& first() {
        return data[0];
    }

    T& second() {
        return data[1];
    }

    T first() const {
        return data[0];
    }

    T second() const {
        return data[1];
    }
};


class AlienData {

private:
    std::variant<std::vector<AlienData>, int64_t, VectorPair<AlienData>> data;
    
public:
    AlienData(const std::vector<AlienData>& vector) : data(vector) { }

    AlienData(int64_t number) : data(number) { }

    AlienData(const VectorPair<AlienData>& pair) : data(pair) { }

    std::vector<AlienData>& getVector() {
        return std::get<std::vector<AlienData>>(data);
    }

    int64_t& getNumber() {
        return std::get<int64_t>(data);
    }

    VectorPair<AlienData>& getPair() {
        return std::get<VectorPair<AlienData>>(data);
    }

    std::vector<AlienData> getVector() const {
        return std::get<std::vector<AlienData>>(data);
    }

    VectorPair<AlienData> getPair() const {
        return std::get<VectorPair<AlienData>>(data);
    }

    int64_t getNumber() const {
        return std::get<int64_t>(data);
    }

    bool isVector() const {
        return std::holds_alternative<std::vector<AlienData>>(data);
    }

    bool isNumber() const {
        return std::holds_alternative<int64_t>(data);
    }

    bool isPair() const {
        return std::holds_alternative<VectorPair<AlienData>>(data);
    }


    std::string to_string() const {
        if (isNumber()) {
            return std::to_string(getNumber());
        } else if (isVector()) {
            std::string result = "[";
            bool first = true;
            for (const auto& element : getVector()) {
                if (!first) {
                    result += ", ";
                }
                first = false;
                result += element.to_string();
            }
            result += ']';
            return result;
        } else {
            return "(" + getPair().first().to_string() + ", " + getPair().second().to_string() + ")";
        }
    }
};
