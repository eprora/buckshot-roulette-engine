#pragma once
#include "engine/agents/intelligent_agent.hpp"

namespace engine{
    class AutomaticIntelligentAgent : public IntelligentAgent {
    public:
        using State = IntelligentAgent::State;
        using Evaluator = IntelligentAgent::Evaluator;
        using StateMachine = IntelligentAgent::StateMachine;
        using Search = IntelligentAgent::Search;

        AutomaticIntelligentAgent(const double time_limit = parameters::TIME_LIMIT, const bool activate_logging = false) {this->logging = activate_logging; this->time_limit = time_limit;}
        State getSuccessor(State state, std::vector<std::unique_ptr<State>> children) override;
        void confirm() const override { return; }
        void reset() override{ last_result = {}; }

        bool logging{false};
    };
}