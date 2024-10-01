#pragma once
#include <cassert>
#include <random>
#include <tuple>
#include <iostream>
#include "engine/item_drawers/item_drawer.hpp"
#include "engine/objects/types.hpp"

namespace engine{

    class RandomizedItemDrawer : public ItemDrawer {
    private:
        std::mt19937 random_number_generator;
    public:
        void setSeed(unsigned int seed) override { random_number_generator = std::mt19937(seed); }
        std::pair<std::vector<engine::Item>, std::vector<engine::Item>> getItems(const unsigned int max_health, std::vector<engine::Item> player_items, std::vector<engine::Item> dealer_items) override;
    
    private:
        engine::Item getRandomItem(const bool without_handsaw);
    };
}