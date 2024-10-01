#include "engine/agents/interactive_agent.hpp"
#include "string_functions.hpp"
#include <limits>

namespace{   
    void displayItems(const std::vector<engine::Item>& items) {
        for (const auto& item : items) {
            std::cout << engine::toString(item) << " ";
        }
        std::cout << "\n";
    }
}

namespace engine{

    engine::State InteractiveAgent::getSuccessor(State state, std::vector<std::unique_ptr<engine::State>> children) {
        // list options
        std::cout << "Dealer's items: ";
        displayItems(state.dealer.items);
        std::cout << "Here are the " << children.size() << " options: \n";
        for (unsigned int i = 0; i < children.size(); ++i) {
            std::cout << i << ": " << toString(children[i]->next_event) << "\n";
        }
        std::cout << "Enter the chosen option: \n";
        unsigned int option;
        do {
            std::cin >> option;
            std::cin.sync();
            std::cin.clear();
        } while(option >= children.size());
        return *children[option];
    }

    void InteractiveAgent::confirm() const{
        std::cout << "Confirm to continue.\n";
        std::cin.get(); // let user confirm this
        std::cin.sync();
        std::cin.clear();
    }
}