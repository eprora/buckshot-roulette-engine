#pragma once

#include <bitset>
#include <cstdint>

namespace engine{
    enum class Item : unsigned int{
        None,
        Cigarette,
        Glass,
        Saw,
        Handcuffs,
        Phone,
        Beer,
        Pills,
        Inverter,
        Adrenalin,
        Count
    };
    
    enum class Round : unsigned int{
        Unknown,
        BlankRound,
        LiveRound
    };

    struct RoundKnowledge {
        Round true_state{Round::Unknown};
        bool player_knowledge{false};
        bool dealer_knowledge{false};
        bool possible_dealer_knowledge{false};

        bool getHash() const{
            return player_knowledge ^ dealer_knowledge ^ (true_state != Round::LiveRound);
        }

        bool operator==(const RoundKnowledge& other) const {
            if (player_knowledge != other.player_knowledge || dealer_knowledge != other.dealer_knowledge || true_state != other.true_state) {
                return false;
            }
            return true;
        }

        bool operator!=(const RoundKnowledge& other) const {
            return !(*this == other);
        }
    };
    
    enum class Action : unsigned int{
        Evaluating,
        ShootSelf,
        ShootOther,
        UseItem
    };

    struct Event{
        bool is_player_turn{true};
        Action action{Action::Evaluating};
        Item item{Item::None};

        bool operator==(const Event& other) const {
            if (is_player_turn != other.is_player_turn || action != other.action) {
                return false;
            }
            if (action == Action::UseItem && item != other.item) {
                return false;
            }
            return true;
        }

        bool operator!=(const Event& other) const {
            return !(*this == other);
        }

        std::pair<std::bitset<32>, uint32_t> getHash() const{

            // leading two bits for the lives
            std::bitset<32> hash;
            if(is_player_turn) {
                hash.set(0);
            }
            if(action == Action::Evaluating) {
                hash.set(1);
            }

            return {hash, 2};
        }
    };
}