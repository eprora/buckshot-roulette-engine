#include "engine/item_drawers/randomized_item_drawer.hpp"
#include "string_functions.hpp"
#include "engine/game_parameters.hpp"
#include <limits>
#include <random>

namespace {
    std::uniform_int_distribution<> draw_size_distribution(game_parameters::MIN_ITEM_DRAW, game_parameters::MAX_ITEM_DRAW);
    std::uniform_int_distribution<> item_distribution(1, static_cast<int>(engine::Item::Count) - 1);
    static constexpr std::array<int, 10> max_amounts{0, 1, 3, 3, 1, 1, 2, 1, 8, 2};
}

namespace engine{
    std::pair<std::vector<engine::Item>, std::vector<engine::Item>> RandomizedItemDrawer::getItems(const unsigned int max_health, std::vector<engine::Item> player_items, std::vector<engine::Item> dealer_items) {
        const std::size_t max_number_of_items_to_draw = draw_size_distribution(random_number_generator);
        const bool without_handsaw = max_health < 3;

        // count the items
        std::array<int, 10> player_amounts{0};
        for(const auto item : player_items) ++player_amounts[static_cast<std::size_t>(item)];
        std::array<int, 10> dealer_amounts{0};
        for(const auto item : dealer_items) ++dealer_amounts[static_cast<std::size_t>(item)];

        unsigned int player_items_to_draw = std::min(max_number_of_items_to_draw, game_parameters::MAX_SLOTS - player_items.size());
        for(unsigned int index = 0; index < player_items_to_draw; ++index){
            engine::Item item;
            do {
                item = getRandomItem(without_handsaw);
            } while (player_amounts[static_cast<std::size_t>(item)] >= max_amounts[static_cast<std::size_t>(item)]);
            ++player_amounts[static_cast<std::size_t>(item)];
            player_items.push_back(item);
        }
        unsigned int dealer_items_to_draw = std::min(max_number_of_items_to_draw, game_parameters::MAX_SLOTS - dealer_items.size());
        for(unsigned int index = 0; index < dealer_items_to_draw; ++index){
            engine::Item item;
            do {
                item = getRandomItem(without_handsaw);
            } while (dealer_amounts[static_cast<std::size_t>(item)] >= max_amounts[static_cast<std::size_t>(item)]);
            ++dealer_amounts[static_cast<std::size_t>(item)];
            dealer_items.push_back(item);
        }
        assert(player_items.size() <= game_parameters::MAX_SLOTS);
        assert(dealer_items.size() <= game_parameters::MAX_SLOTS);

        return {std::move(player_items),std::move(dealer_items)};
    }

    engine::Item RandomizedItemDrawer::getRandomItem(const bool without_handsaw) {
        while(true) {
            const engine::Item item = static_cast<engine::Item>(item_distribution(random_number_generator));
            if((item == engine::Item::Saw) && without_handsaw) continue;
            return item;
        }
    }
}
