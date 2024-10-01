#pragma once
#include <cassert>
#include <vector>
#include <iostream>
#include "engine/item_drawers/item_drawer.hpp"
#include "engine/item_drawers/item_drawer.hpp"

namespace engine{
    class GetInputItemDrawer : public ItemDrawer {
    public:
        void setSeed(unsigned int seed) override { return; }
        std::pair<std::vector<engine::Item>, std::vector<engine::Item>> getItems(const unsigned int max_health, std::vector<engine::Item> player_items, std::vector<engine::Item> dealer_items) override;
    };
}