#include "string_functions.hpp"

namespace engine{
    std::string toString(const engine::Action action) {
        switch (action) {
            case engine::Action::Evaluating: return "evaluate";
            case engine::Action::ShootSelf: return "shoot self";
            case engine::Action::ShootOther: return "shoot other person";
            case engine::Action::UseItem: return "use";
            default: return "Unknown";
        }
    }

    std::string toString(const engine::Item item) {
        switch (item) {
            case engine::Item::None: return "nothing";
            case engine::Item::Cigarette: return "cigarette";
            case engine::Item::Beer: return "beer";
            case engine::Item::Pills: return "pills";
            case engine::Item::Saw: return "saw";
            case engine::Item::Handcuffs: return "handcuffs";
            case engine::Item::Glass: return "glass";
            case engine::Item::Phone: return "phone";
            case engine::Item::Inverter: return "inverter";
            case engine::Item::Adrenalin: return "adrenalin";
            default: return "Unknown";
        }
    }

    std::string toString(const engine::Event& event) {
        std::string result = std::string(event.is_player_turn ? "You will " : "Dealer will ") + toString(event.action);
        if(event.action == engine::Action::UseItem) result += " " + toString(event.item);
        return result;
    }

    void visualizeTreeHelper(const engine::State& node, unsigned int depth, std::string& result) {
        // Add the current node to the result
        result += std::string(depth, '-') + toString(node.next_event) + "\n";
        
        // Get the children of the current node
        if(StateMachine::isFinished(node)) return;
        auto children = StateMachine::getChildStates(node);
        
        if(children.empty()) {
            throw std::runtime_error(toString(node.next_event) + " has no children.\n");
            return;
        }
        
        // Recursively add the children to the result
        for (const auto& child : children) {
            visualizeTreeHelper(*child, depth + 1, result);
        }
    }

    std::string visualizeTree(const engine::State& root) {
        std::string result;
        visualizeTreeHelper(root, 0, result);
        return result;
    }
}