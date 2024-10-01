#include "engine/item_drawers/get_input_item_drawer.hpp"
#include "string_functions.hpp"
#include <limits>
#include <random>

namespace {
    void displayItems(const std::vector<engine::Item>& items) {
        for (const auto& item : items) {
            std::cout << engine::toString(item) << " ";
        }
        std::cout << "\n";
    }
    
    void displayItemOptions() {
        std::cout << "Enter the item type: (None if finished)\n";
        for (int i = 0; i < static_cast<int>(engine::Item::Count); ++i) {
            std::cout << i << ": " << engine::toString(static_cast<engine::Item>(i)) << "\n";
        }
    }

    engine::Item getItemFromInput() {
        unsigned int item;
        do {
            std::cin >> item;
            std::cin.sync();
            std::cin.clear();
        } while (item >= static_cast<unsigned int>(engine::Item::Count));
        return static_cast<engine::Item>(item);
    }
}

namespace engine{
    std::pair<std::vector<engine::Item>, std::vector<engine::Item>> GetInputItemDrawer::getItems(const unsigned int max_health, std::vector<engine::Item> player_items, std::vector<engine::Item> dealer_items){
        while(true) {
            player_items.clear();
            dealer_items.clear();

            std::cout << "Drawing items for the player...\n";
            displayItemOptions();
            for (unsigned int i = 0; i < 8; ++i) {
                Item item = getItemFromInput();
                if (item == Item::None) break;
                player_items.push_back(item);
            }

            std::cout << "Drawing items for the dealer...\n";
            for (unsigned int i = 0; i < 8; ++i) {
                Item item = getItemFromInput();
                if (item == Item::None) break;
                dealer_items.push_back(item);
            }

            std::cout << "Player's items: ";
            displayItems(player_items);

            std::cout << "Dealer's items: ";
            displayItems(dealer_items);

            std::string confirmation;
            std::cout << "Is everything correct? (+/-): ";
            std::cin >> confirmation;
            std::cin.sync();
            std::cin.clear();
            if(confirmation == "+") break;
        }
        return {std::move(player_items), std::move(dealer_items)};
    }
}
