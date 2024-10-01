#pragma once

#include <deque>
#include <memory>
#include <cassert>
#include <stdexcept>
#include <limits>
#include "parameters.hpp"
#include <unordered_map>
#include <list>
#include <mutex>

namespace search{
    template <typename StateMachineType, typename EvaluatorType>
    class TranspositionSearch {
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

        void update_cache(const State& state, const double result, const uint32_t depth);

    private:
        size_t max_cache_size{5000000};
        std::unordered_map<State, std::tuple<double, uint32_t>> transposition_table;
        std::mutex table_mutex;
    };

    template <typename StateMachineType, typename EvaluatorType>
    std::atomic<bool> TranspositionSearch<StateMachineType, EvaluatorType>::timeout{false};

    template <typename StateMachineType, typename EvaluatorType>
    void TranspositionSearch<StateMachineType, EvaluatorType>::update_cache(const State& state, const double result, const uint32_t depth) {
        std::lock_guard<std::mutex> lock(table_mutex);
        transposition_table[state] = {result, depth};
    }

    template <typename StateMachineType, typename EvaluatorType>
    double TranspositionSearch<StateMachineType, EvaluatorType>::expectiminimax(const State& parent, const uint32_t depth, double alpha, double beta) {
        if(timeout) throw std::runtime_error("timeout");

        // terminal nodes
        if(StateMachine::isFinished(parent) || !depth) {
            return Evaluator::getScore(parent);
        }

        {
            std::lock_guard<std::mutex> lock(table_mutex);
            auto it = transposition_table.find(parent);
            if (it != transposition_table.end() && std::get<1>(it->second) >= depth) {
                return std::get<0>(it->second);
            }
        }
        // get children:
        auto children = StateMachine::getChildStates(parent);
        assert(!children.empty());
        if(children.size() == 1) {
            // skip single childs in depth computation
            return expectiminimax(*children.front(), depth, alpha, beta);
        }
        const bool is_evaluation = StateMachine::isEvaluationPhase(parent.next_event);

        double end_result;
        if(is_evaluation) {
            const bool is_player_turn = StateMachine::isPlayerTurn(parent);
            end_result = is_player_turn ? -std::numeric_limits<double>::infinity() : std::numeric_limits<double>::infinity();

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
        } else {
            // random event happens
            end_result = 0.0;
            double total_probability = 0.0;
            for( const auto& child : children ) {
                // accumulate results
                const double result = expectiminimax(*child, depth-1, alpha, beta);
                total_probability += child->probability;
                end_result += child->probability * result;
            }
            assert(std::abs(total_probability - 1.0) < parameters::EPSILON);
        }

        update_cache(parent, end_result, depth);
        return end_result;
    }
}