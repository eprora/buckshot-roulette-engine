#pragma once
#include <tuple>
#include <vector>
#include "engine/objects/types.hpp"

namespace engine{
    class ItemDrawer {
    public:
        virtual ~ItemDrawer() = default;
        virtual void setSeed(unsigned int seed) = 0;
        virtual std::pair<std::vector<engine::Item>, std::vector<engine::Item>> getItems(const unsigned int max_health, std::vector<engine::Item> player_items, std::vector<engine::Item> dealer_items) = 0;
    };
}