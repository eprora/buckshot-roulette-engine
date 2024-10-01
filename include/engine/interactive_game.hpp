#pragma once

#include "engine/game.hpp"
#include <string>

namespace engine{
    class InteractiveGame : private Game{
    public:
        InteractiveGame() : Game(createRandomizer(), createPlayer(), createDealer(), createItemDrawer(), createSeed(), true){}
        void start();
        void playMove();
        bool isFinished() const;

    private:
        std::string mode{""};

        void startPredetermined();
        static std::unique_ptr<randomizer::Randomizer<State>> createRandomizer();
        static std::unique_ptr<engine::Agent> createPlayer();
        static std::unique_ptr<engine::Agent> createDealer();
        static std::unique_ptr<engine::ItemDrawer> createItemDrawer();
        static unsigned int createSeed();
    };
}