#pragma once
#include "engine/agents/intelligent_agent.hpp"
#include "engine/agents/interactive_agent.hpp"

namespace engine{

    class InteractiveIntelligentAgent : private IntelligentAgent, public InteractiveAgent {
    public:
        InteractiveIntelligentAgent(const double time_limit = parameters::TIME_LIMIT) {this->time_limit = time_limit;}
        State getSuccessor(State state, std::vector<std::unique_ptr<State>> children) override;
        void reset() override{ last_result = {}; }
    };
}