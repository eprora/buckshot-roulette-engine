#pragma once
#include "engine/agents/agent.hpp"
#include "search/iterative_search.hpp"
#include "search/threaded_search.hpp"
#include "search/transposition_search.hpp"
#include "engine/state_machine.hpp"
#include "engine/evaluator.hpp"
#include "parameters.hpp"
#include "string_functions.hpp"

namespace engine{
    class IntelligentAgent : virtual public Agent {
    public:
        using State = engine::State;
        using Evaluator = engine::Evaluator;
        using StateMachine = engine::StateMachine;
        using Search = search::ThreadedSearch<search::IterativeSearch<search::TranspositionSearch<StateMachine, Evaluator>>>;

    protected:
        Search::Result last_result{};
        double time_limit{parameters::TIME_LIMIT};
        Search::Result getBestChoice(const State& state, const bool logging) const;
    };
}