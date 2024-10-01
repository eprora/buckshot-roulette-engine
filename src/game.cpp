#include "engine/game.hpp"
#include "engine/game_parameters.hpp"
#include "string_functions.hpp"

namespace {
    std::uniform_int_distribution<> life_distribution(game_parameters::MIN_LIVES, game_parameters::MAX_LIVES);
    std::uniform_int_distribution<> shell_distribution(game_parameters::MIN_SHELLS, game_parameters::MAX_SHELLS);
}

namespace engine{

    void Game::startRandomized(){
        const bool start_new_game = !current_state.player.lives || !current_state.dealer.lives;
        unsigned int lives = start_new_game ? life_distribution(random_number_generator) : 0;
        unsigned int num_rounds = shell_distribution(random_number_generator);
        unsigned int live_rounds = std::max(1U, num_rounds / 2U);
        unsigned int blank_rounds = num_rounds - live_rounds;

        Game::start(live_rounds, blank_rounds, lives);
        player->confirm();
    }

    void Game::start(const unsigned int live_rounds, const unsigned int blank_rounds, const unsigned int lives){
        if(lives) {
            current_state.resetLives(lives);
            current_state.player.items.clear();
            current_state.dealer.items.clear();
        }
        current_state.shotgun.load(live_rounds, blank_rounds);
        current_state.handcuffs.remove();
        current_state.next_event = {true, Action::Evaluating, Item::None};
        player->reset();
        dealer->reset();
        std::tie(current_state.player.items, current_state.dealer.items) = item_drawer->getItems(current_state.max_lives, std::move(current_state.player.items), std::move(current_state.dealer.items));
    }

    void Game::playMove() {
        current_state = getSuccessor(current_state);
    }

    State Game::getSuccessor(State state) const{
        auto children = StateMachine::getChildStates(state);

        // random events
        if(!StateMachine::isEvaluationPhase(state.next_event)) {
            auto result = getSuccessorAfterEvent(std::move(state), std::move(children));
            informPlayer();
            return std::move(result);
        }

        // player/dealer choice
        const bool is_player_turn = StateMachine::isPlayerTurn(state);
        if(is_player_turn) {
            return player->getSuccessor(std::move(state), std::move(children));
        } else {
            auto result = dealer->getSuccessor(std::move(state), std::move(children));
            informPlayer();
            return std::move(result);
        }
    }

    State Game::getSuccessorAfterEvent(State state, std::vector<std::unique_ptr<State>> children) const{
        assert(!children.empty());
        const auto item = state.next_event.item;
        const auto action = state.next_event.action;
        if(children.size() == 1) {
            // always choose obvious option
            // this is the case for:
            // when the loaded round is certain
            // but exclude the obvious options
            assert(!(action == Action::UseItem && item == Item::Saw));
            assert(!(action == Action::UseItem && item == Item::Cigarette));
            assert(!(action == Action::UseItem && item == Item::Handcuffs));
            assert(!(action == Action::UseItem && item == Item::Adrenalin));
            if(logging) std::cout << "The only possible outcome is: " << toString(children.front()->next_event) << "\n";
            return std::move(*children.front());
        }

        // possible items to land in this part of code:
        // glass, phone, beer, pills, inverter
        if(action == Action::UseItem &&
           (item == Item::Glass || item == Item::Phone) &&
           !StateMachine::isPlayerTurn(state)) {
            // player can't tell the result here
            return randomizer->getHiddenKnowledgeSuccessor(std::move(state), std::move(children), item == Item::Phone);
        } else if(action == Action::UseItem && item == Item::Phone) {
            return getSuccessorAfterPhone(std::move(state), std::move(children));
        } else if(action == Action::UseItem && item == Item::Pills) {
            return getSuccessorAfterPills(std::move(state), std::move(children));
        } else {
            // possible scenarios to land in this part of code:
            // glass, beer, any shooting option
            return getSuccessorAfterShellEjection(std::move(state), std::move(children));
        }
    }
    
    State Game::getSuccessorAfterPhone(State state, std::vector<std::unique_ptr<State>> children) const{
        const auto left_rounds = state.shotgun.getRemainingRounds();
        unsigned int available_option = 0;
        
        // enumerate all options
        if(logging) std::cout << "There are " << left_rounds << " left rounds.\n";
        for(unsigned int index = 1; index < left_rounds; ++index) {
            const auto probability = StateMachine::getProbabilityOfBlankRound(state, children.front()->inverter_used, index);
            if(probability > parameters::EPSILON) {
                if(logging) std::cout << available_option << ": Shell " << index + 1 << ": Blank round.\n";
                ++available_option;
            }
            if(1.0 - probability > parameters::EPSILON) {
                if(logging) std::cout << available_option << ": Shell " << index + 1 << ": Live round.\n";
                ++available_option;
            }
        }

        // choose option
        assert(children.size() == available_option);
        return randomizer->getSuccessor(std::move(children));
    }

    State Game::getSuccessorAfterPills(State state, std::vector<std::unique_ptr<State>> children) const{
        if(logging) std::cout << "What happened after taking the pills?\n";
        if(logging) std::cout << "0: die, 1: live\n";
        assert(children.size() == 2);
        return randomizer->getSuccessor(std::move(children));
    }

    State Game::getSuccessorAfterShellEjection(State state, std::vector<std::unique_ptr<State>> children) const{
        assert(children.size() == 2);
        if(logging) std::cout << "What round type did you observe?\n";
        if(logging) std::cout << "0: blank, 1: live round\n";
        if(state.inverter_used) std::swap(children.front(), children.back());
        return randomizer->getSuccessor(std::move(children));
    }

    void Game::informPlayer() const{
        player->confirm();
    }

    bool Game::isFinished() const{
        return StateMachine::isFinished(current_state);
    }

    bool Game::isWon() const{
        return !current_state.dealer.lives;
    }

    bool Game::isLost() const{
        return !current_state.player.lives;
    }
}