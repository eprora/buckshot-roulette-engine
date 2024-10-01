#pragma once

#include <vector>
#include <memory>
#include <queue>
#include <thread>
#include <future>
#include <algorithm>
#include "parameters.hpp"

namespace search{
    template <typename BaseSearch>
    class IterativeSearch : public BaseSearch {
    public:
        using Evaluator = typename BaseSearch::Evaluator;
        using StateMachine = typename BaseSearch::StateMachine;
        using State = typename StateMachine::State;
        using Event = typename StateMachine::Event;
        using Result = typename BaseSearch::Result;

        /// @brief Performs the minimax algorithm with threading
        /// @param parent state to evaluate
        /// @param depth max shallow depth to evaluate
        /// @param alpha lower bound for alpha-beta pruning
        /// @param beta upper bound for alpha-beta pruning
        /// @return best score that the parent gets
        Result expectiminimax(const State& parent, const uint32_t depth, double alpha = -std::numeric_limits<double>::infinity(), double beta = std::numeric_limits<double>::infinity());
    };

    template <typename BaseSearch>
    typename IterativeSearch<BaseSearch>::Result IterativeSearch<BaseSearch>::expectiminimax(const State& parent, const uint32_t depth, double alpha, double beta) {
        
        Result end_result;
        BaseSearch::timeout.store(false);
        std::chrono::duration<double> elapsed;
        for (unsigned int iterative_depth = 1; iterative_depth <= depth; ++iterative_depth) {
            try {
                end_result = BaseSearch::expectiminimax(parent, iterative_depth, alpha, beta);
            } catch (...) {
                return end_result;
            }
        }

        return end_result;
    }
}
