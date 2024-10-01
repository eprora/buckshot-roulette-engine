#pragma once

#include <deque>
#include <memory>
#include <cassert>
#include <stdexcept>
#include <limits>
#include "parameters.hpp"

namespace search{
    template <typename StateMachineType, typename EvaluatorType>
    class Search {
    public:
        using StateMachine = StateMachineType;
        using Evaluator = EvaluatorType;
        using State = typename StateMachine::State;
        using Event = typename StateMachine::Event;
        using Result = double;

        // set the timeout to stop evaluation immediately
        static std::atomic<bool> timeout;

        /// @brief Performs the minimax algorithm only to find the score of the parent
        /// @param parent state to evaluate
        /// @param depth max depth to evaluate
        /// @param alpha lower bound for alpha-beta pruning
        /// @param beta upper bound for alpha-beta pruning
        /// @return best score that the parent gets
        double expectiminimax(const State& parent, const uint32_t depth, double alpha = -std::numeric_limits<double>::infinity(), double beta = std::numeric_limits<double>::infinity());
    };

    template <typename StateMachineType, typename EvaluatorType>
    std::atomic<bool> Search<StateMachineType, EvaluatorType>::timeout{false};

    template <typename StateMachineType, typename EvaluatorType>
    double Search<StateMachineType, EvaluatorType>::expectiminimax(const State& parent, const uint32_t depth, double alpha, double beta){
        if(timeout) throw std::runtime_error("timeout");

        // terminal nodes
        if(StateMachine::isFinished(parent) || !depth) {
            return Evaluator::getScore(parent);
        }
        // get children:
        auto children = StateMachine::getChildStates(parent);
        assert(!children.empty());
        if(children.size() == 1) {
            // skip single childs in depth computation
            return expectiminimax(*children.front(), depth, alpha, beta);
        }
        const bool is_evaluation = StateMachine::isEvaluationPhase(parent.next_event);

        if(is_evaluation) {
            const bool is_player_turn = StateMachine::isPlayerTurn(parent);
            double end_result = is_player_turn ? -std::numeric_limits<double>::infinity() : std::numeric_limits<double>::infinity();

            for( auto& child : children ) {
                const auto result = expectiminimax(*child, depth-1, alpha, beta);
                if(is_player_turn) {
                    if (result > end_result) {
                        end_result = result;
                    }
                    // alpha-beta pruning
                    if (end_result > beta) break;
                    alpha = std::max(alpha, end_result);
                } else {
                    if (result < end_result) {
                        end_result = result;
                    }
                    // alpha-beta pruning
                    if (end_result < alpha) break;
                    beta = std::min(beta, end_result);
                }
            }
            return end_result;
        } else {
            // random event happens
            double end_result = 0.0;
            double total_probability = 0.0;
            for( const auto& child : children ) {
                // accumulate results
                const double result = expectiminimax(*child, depth-1, alpha, beta);
                total_probability += child->probability;
                end_result += child->probability * result;
            }
            assert(std::abs(total_probability - 1.0) < parameters::EPSILON);
            return end_result;
        }
    }
}
