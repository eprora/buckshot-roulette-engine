#include "engine/interactive_game.hpp"
#include "string_functions.hpp"
#include "engine/agents/automatic_intelligent_agent.hpp"
#include "engine/agents/interactive_intelligent_agent.hpp"
#include "engine/item_drawers/randomized_item_drawer.hpp"
#include "engine/item_drawers/get_input_item_drawer.hpp"

namespace {
    void displayItems(const std::vector<engine::Item>& items) {
        for (const auto& item : items) {
            std::cout << engine::toString(item) << " ";
        }
        std::cout << "\n";
    }
}

namespace engine{
    std::unique_ptr<randomizer::Randomizer<State>> InteractiveGame::createRandomizer() {
        std::string input;
        std::cout << "Wanna use randomized outcomes? (+/-) ";
        std::cin >> input;
        std::cin.sync();
        std::cin.clear();
        if(input == "+") {
            std::cout << "A random outcome is chosen at every stage.\n";
            return std::make_unique<randomizer::TrueRandomizer<State>>(true);
        } else {
            std::cout << "You must provide the observed outcomes.\n";
            return std::make_unique<randomizer::GetInputRandomizer<State>>(true);
        }
    }

    std::unique_ptr<engine::Agent> InteractiveGame::createPlayer() {
        std::string input;
        std::cout << "Wanna get recommendations? (+/-) ";
        std::cin >> input;
        std::cin.sync();
        std::cin.clear();
        if(input == "+") {
            std::cout << "You get recommendation for the best move.\n";
            double input_time;
            std::cout << "Set the time limit for evaluation (seconds): ";
            std::cin >> input_time;
            std::cin.sync();
            std::cin.clear();
            return std::make_unique<InteractiveIntelligentAgent>(input_time);
        } else {
            std::cout << "You get no help.\n";
            return std::make_unique<InteractiveAgent>();
        }
    }

    std::unique_ptr<engine::Agent> InteractiveGame::createDealer() {
        std::string input;
        std::cout << "Wanna play against computer? (+/-) ";
        std::cin >> input;
        std::cin.sync();
        std::cin.clear();
        if(input == "+") {
            std::cout << "Dealer choses their best move.\n";
            double input_time;
            std::cout << "Set the time limit for evaluation (seconds): ";
            std::cin >> input_time;
            std::cin.sync();
            std::cin.clear();
            return std::make_unique<AutomaticIntelligentAgent>(input_time, true);
        } else {
            std::cout << "You must provide input on which option the dealer choses.\n";
            return std::make_unique<InteractiveAgent>();
        }
    }

    std::unique_ptr<engine::ItemDrawer> InteractiveGame::createItemDrawer() {
        std::string input;
        std::cout << "Wanna have randomized items? (+/-) ";
        std::cin >> input;
        std::cin.sync();
        std::cin.clear();
        if(input == "+") {
            std::cout << "Items are drawn randomly according to the game rules.\n";
            return std::make_unique<RandomizedItemDrawer>();
        } else {
            std::cout << "You must provide input on the existing items.\n";
            return std::make_unique<GetInputItemDrawer>();
        }
    }

    unsigned int InteractiveGame::createSeed(){
        std::cout << "If you want to set a seed put in a nonzero number,\n"
                      "In case of zero the seed is random: ";
        unsigned int seed;
        std::cin >> seed;
        std::cin.sync();
        std::cin.clear();
        if(!seed) {
            std::random_device device;
            seed = device();
        }
        std::cout << "seed used: " << seed << "\n";
        return seed;
    }

    void InteractiveGame::start(){
        while(true) {
            if(mode == "+") {
                std::cout << "Configuration is chosen randomly according to the game rules.\n";
                startRandomized();
            } else if(mode == "-") {
                std::cout << "You must provide input on the configuration.\n";
                startPredetermined();
            } else {
                std::cout << "Wanna have randomized lives and shotgun shells? (+/-) ";
                std::cin >> mode;
                std::cin.sync();
                std::cin.clear();
                continue;
            }

            const auto& current_state = getCurrentState();
            std::cout << "\nNEW GAME --------------------------------------------------\n";
            std::cout << "live rounds: " << current_state.shotgun.getRemainingLiveRounds() << ", blank rounds: " <<  current_state.shotgun.getRemainingBlankRounds()  << "\n";
            std::cout << "dealer's lives: " << current_state.dealer.lives; 
            std::cout << ", player's lives: " <<  current_state.player.lives << "\n";
            std::cout << "dealer's items: ";
            displayItems(current_state.dealer.items);
            std::cout << "player's items: ";
            displayItems(current_state.player.items);
            return;
        }
    }

    void InteractiveGame::startPredetermined(){
        unsigned int lives, live_rounds, blank_rounds;
        std::string confirmation;
        
        std::cout << "Enter the number of lives (zero to continue current stage): ";
        std::cin >> lives;
        std::cin.sync();
        std::cin.clear();

        std::cout << "Enter the number of live rounds: ";
        std::cin >> live_rounds;
        std::cin.sync();
        std::cin.clear();

        std::cout << "Enter the number of blank rounds: ";
        std::cin >> blank_rounds;
        std::cin.sync();
        std::cin.clear();

        std::cout << "Is everything correct? (+/-): ";
        std::cin >> confirmation;
        std::cin.sync();
        std::cin.clear();

        Game::start(live_rounds, blank_rounds, lives);
    }

    void InteractiveGame::playMove() {
        const auto& current_state = getCurrentState();
        std::cout << "\nNEW MOVE ---------------------------------------------------\n";
        const auto rounds_left = current_state.shotgun.getRemainingRounds();
        std::cout << "Rounds left: " << rounds_left << ".\n";
        std::cout << "Next event: " << toString(current_state.next_event) << ".\n";
        Game::playMove();
        const auto& new_state = getCurrentState();
        std::cout << "dealer's lives: " << new_state.dealer.lives; 
        std::cout << ", player's lives: " <<  new_state.player.lives << "\n";
        std::cout << "Move finished.\n";
    }

    bool InteractiveGame::isFinished() const{
        if(!Game::isFinished()) return false;
        if(Game::isWon()) std::cout << "Player won.\n";
        if(Game::isLost()) std::cout << "Dealer won.\n";
        std::cout << "Game finished.----------------------------------------------\n";
        return true;
    }
}