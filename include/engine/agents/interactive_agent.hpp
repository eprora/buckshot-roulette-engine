#pragma once
#include "engine/agents/agent.hpp"

namespace engine{

    class InteractiveAgent : virtual public Agent {
    public:
        engine::State getSuccessor(State state, std::vector<std::unique_ptr<engine::State>> children) override;
        void confirm() const override;
        void reset() override{ return; }
    };
}