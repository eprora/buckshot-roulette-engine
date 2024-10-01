#pragma once
#include "engine/objects/state.hpp"

#include <cassert>
#include <numeric>
#include <iostream>

namespace engine{
    // Base class Agent
    class Agent {
    public:
        virtual ~Agent() = default;
        virtual engine::State getSuccessor(State state, std::vector<std::unique_ptr<engine::State>> children) = 0;
        virtual void confirm() const = 0;
        virtual void reset() = 0;
    };
}