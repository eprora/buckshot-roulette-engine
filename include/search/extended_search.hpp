#pragma once

#include <deque>
#include <memory>
#include <cassert>
#include <stdexcept>
#include <limits>
#include "parameters.hpp"

namespace search{

    template <typename BaseSearch>
    class ExtendedSearch : public BaseSearch {
    public:
        using Evaluator = typename BaseSearch::Evaluator;
        using StateMachine = typename BaseSearch::StateMachine;
        using State = typename StateMachine::State;
        using Event = typename StateMachine::Event;

        struct Result{
            std::deque<Event> follow_ups{}; // all choices until next random event
            double score {0.0};

            bool operator==(const Result& other) const {
                if (std::abs(score - other.score) > parameters::EPSILON) return false;
                if (follow_ups.size() !=  other.follow_ups.size()) return false;
                for(uint32_t idx = 0; idx < follow_ups.size(); ++idx) {
                    if(follow_ups[idx] != other.follow_ups[idx]) return false;
                }
                return true;
            }

            bool operator!=(const Result& other) const {
                return !(*this == other);
            }
        };

        /// @brief Performs the minimax algorithm to find the best choice of action for 
        /// @param parent state to evaluate
        /// @param depth max depth to evaluate with extended search
        /// @param deep_depth after performing extended search perform faster search
        /// @param alpha lower bound for alpha-beta pruning
        /// @param beta upper bound for alpha-beta pruning
        /// @return A combination of the best choice of action, the score of that result and the total number of evaluated states
        
        Result expectiminimax(const State& parent, const uint32_t depth, const uint32_t deep_depth = 0, double alpha = -std::numeric_limits<double>::infinity(), double beta = std::numeric_limits<double>::infinity());
    };

    template <typename BaseSearch>
    typename ExtendedSearch<BaseSearch>::Result ExtendedSearch<BaseSearch>::expectiminimax(const State& parent, const uint32_t depth, const uint32_t deep_depth, double alpha, double beta){
        // terminal nodes
        if(StateMachine::isFinished(parent) || !depth) {
            if(deep_depth) return Result{{}, BaseSearch::expectiminimax(parent, deep_depth, alpha, beta)};
            return Result{{}, Evaluator::getScore(parent)};
        }
        // get children:
        Result end_result;
        const bool is_evaluation = StateMachine::isEvaluationPhase(parent.next_event);
        auto children = StateMachine::getChildStates(parent);
        assert(!children.empty());
        if(children.size() == 1) {
            // skip single childs in depth computation
            end_result = expectiminimax(*children.front(), depth, deep_depth, alpha, beta);
            const auto& next_event = children.front()->next_event;
            if(next_event.action != engine::Action::Evaluating)
                end_result.follow_ups.push_front(next_event);
            return end_result;
        }

        if(is_evaluation) {
            // current participant will optimize their move
            Event best_event;

            const bool is_player_turn = StateMachine::isPlayerTurn(parent);
            end_result.score = is_player_turn ? -std::numeric_limits<double>::infinity() : std::numeric_limits<double>::infinity();

            for( auto& child : children ) {
                const auto result = expectiminimax(*child, depth-1, deep_depth, alpha, beta);
                if(is_player_turn) {
                    if (result.score > end_result.score) {
                        end_result = std::move(result);
                        best_event = std::move(child->next_event);
                    }
                    // alpha-beta pruning
                    if (end_result.score > beta) break;
                    alpha = std::max(alpha, end_result.score);
                } else {
                    if (result.score < end_result.score) {
                        end_result = std::move(result);
                        best_event = std::move(child->next_event);
                    }
                    // alpha-beta pruning
                    if (end_result.score < alpha) break;
                    beta = std::min(beta, end_result.score);
                }
            }
            end_result.follow_ups.push_front(best_event);
            return end_result;
        } else {
            // random event happens
            double total_probability = 0.0;
            for( const auto& child : children ) {
                // accumulate results
                const Result result = expectiminimax(*child, depth-1, deep_depth, alpha, beta);
                total_probability += child->probability;
                // end_result.follow_ups = result.follow_ups;
                // end_result.follow_ups.push_front(child->next_event);
                end_result.score += child->probability * result.score;
            }
            assert(std::abs(total_probability - 1.0) < parameters::EPSILON);
            return end_result;
        }
    }
}
