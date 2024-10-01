#include "engine/objects/participant.hpp"

namespace engine{
    void Participant::removeItem(Item item){    
        auto it = std::find(items.begin(), items.end(), item);
        if (it != items.end()) {
            items.erase(it);
        } else {
            throw std::runtime_error("No elements of type VariantA found");
        }
    }

    void Participant::loseLife() {
        if(lives) --lives;
    }
    
    void Participant::gainLives(const unsigned int gained_lives, const unsigned int max_lives) {
        const unsigned int new_lives = lives + gained_lives;
        lives = std::min(new_lives, max_lives);
    }

    std::pair<std::bitset<32>, uint32_t> Participant::getHash(const int max_slots) const{

        // leading two bits for the lives
        uint32_t hash = lives << 4;

        // Construct the hash
        for (const auto& item : items) {
            hash = hash ^ static_cast<int>(item);
        }

        // size should be 8 item bits + 9 zero bits + 2 live bits = 19
        return {hash, 6};
    }

    bool Participant::operator==(const Participant& other) const {
        if(lives != other.lives) return false;
        if(items.size() != other.items.size()) return false;
        auto this_items = items;
        std::sort(this_items.begin(), this_items.end());
        auto other_items = other.items;
        std::sort(other_items.begin(), other_items.end());
        for (int i = 0; i < items.size(); ++i) {
            if(this_items[i] != other_items[i]) return false;
        }
        return true;
    }
}