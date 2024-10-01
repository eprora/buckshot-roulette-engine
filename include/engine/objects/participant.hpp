#pragma once

#include <vector>
#include <tuple>
#include <bitset>
#include <stdexcept>
#include <algorithm>
#include "engine/objects/types.hpp"
#include "parameters.hpp"

namespace engine{

    struct Participant{
        unsigned int lives{0};
        std::vector<Item> items{};
        
        void removeItem(Item item);

        void loseLife();

        void gainLives(const unsigned int gained_lives, const unsigned int max_lives);

        std::pair<std::bitset<32>, uint32_t> getHash(const int max_slots) const;
        bool operator==(const Participant& other) const;
    };
}