#pragma once

#include "engine/state_machine.hpp"
#include "engine/evaluator.hpp"
#include "engine/agents/agent.hpp"
#include "engine/item_drawers/item_drawer.hpp"
#include "randomizer.hpp"
#include "parameters.hpp"

namespace engine{
    class Game{
    public:
        Game(std::unique_ptr<randomizer::Randomizer<State>> rand,
         std::unique_ptr<engine::Agent> plyr,
         std::unique_ptr<engine::Agent> deal,
         std::unique_ptr<engine::ItemDrawer> drawer,
         unsigned int seed,
         const bool activate_logging = false)
        : randomizer(std::move(rand)),
          player(std::move(plyr)),
          dealer(std::move(deal)),
          item_drawer(std::move(drawer)),
          random_number_generator(seed),
          logging(activate_logging) {
            randomizer->logging = logging;
            randomizer->setSeed(seed);
            item_drawer->setSeed(seed);
        }

        void startRandomized();
        void start(const unsigned int live_rounds, const unsigned int blank_rounds, const unsigned int lives = 0);
        void playMove();
        bool isFinished() const;
        bool isWon() const;
        bool isLost() const;
        const State& getCurrentState() const { return current_state; }

    private:
        std::mt19937 random_number_generator;
        std::unique_ptr<randomizer::Randomizer<State>> randomizer = nullptr;
        std::unique_ptr<engine::Agent> player = nullptr;
        std::unique_ptr<engine::Agent> dealer = nullptr;
        std::unique_ptr<engine::ItemDrawer> item_drawer = nullptr;

        State getSuccessor(State state) const;
        State getSuccessorAfterEvent(State state, std::vector<std::unique_ptr<State>> children) const;
        State getSuccessorAfterPhone(State state, std::vector<std::unique_ptr<State>> children) const;
        State getSuccessorAfterPills(State state, std::vector<std::unique_ptr<State>> children) const;
        State getSuccessorAfterShellEjection(State state, std::vector<std::unique_ptr<State>> children) const;

        void informPlayer() const;

        State current_state{};
        bool logging = false;
    };
}