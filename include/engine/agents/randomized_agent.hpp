#pragma once
#include "engine/agents/agent.hpp"
#include <random>

namespace engine{

    class RandomizedAgent : public Agent {
    private:
        std::mt19937 random_number_generator;
    public:
        void setSeed(unsigned int seed) { random_number_generator = std::mt19937(seed); }
        engine::State getSuccessor(State state, std::vector<std::unique_ptr<engine::State>> children) override;
        void confirm() const override{ return; }
        void reset() override{ return; }
    };
}