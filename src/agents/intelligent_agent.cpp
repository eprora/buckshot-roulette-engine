#include "engine/agents/intelligent_agent.hpp"

namespace engine{

    IntelligentAgent::Search::Result IntelligentAgent::getBestChoice(const State& state, const bool logging) const{

        const unsigned int max_depth = parameters::MAX_SHALLOW_DEPTH;
        const unsigned int max_deep_depth = std::max(max_depth + 1, StateMachine::getMaxDepth(state));
        Search solver;

        // evaluate best choice
        return solver.expectiminimax(state, max_depth, max_deep_depth, time_limit);
    }
}