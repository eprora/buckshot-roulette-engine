#pragma once

#include "search/extended_search.hpp"

#include <vector>
#include <memory>
#include <queue>
#include <thread>
#include <future>
#include <algorithm>
#include <iostream>

namespace search{
    template <typename BaseSearch>
    class ThreadedSearch : public ExtendedSearch<BaseSearch> {
    public:
        using Evaluator = typename BaseSearch::Evaluator;
        using StateMachine = typename BaseSearch::StateMachine;
        using State = typename StateMachine::State;
        using Event = typename StateMachine::Event;
        using Result = typename ThreadedSearch<BaseSearch>::Result;

        static std::atomic<unsigned int> free_threads;

        /// @brief Performs the minimax algorithm with threading
        /// @param parent state to evaluate
        /// @param depth max shallow depth to evaluate
        /// @param deep_depth max deep depth to evaluate
        /// @param time_limit abort evaluation after the time limit was reached
        /// @return vector of children scores
        std::vector<Result> expectiminimaxThreaded(const std::vector<std::unique_ptr<State>>& children, const uint32_t depth, const uint32_t deep_depth = 0, const double time_limit = parameters::TIME_LIMIT);

        /// @brief Performs the minimax algorithm with threading
        /// @param parent state to evaluate
        /// @param depth max shallow depth to evaluate
        /// @param deep_depth max deep depth to evaluate
        /// @param time_limit abort evaluation after the time limit was reached
        /// @return best score that the parent gets
        Result expectiminimaxSingleLayer(const State& parent, const std::vector<std::unique_ptr<State>>& children, const std::vector<Result>& results);

        /// @brief Performs the minimax algorithm with threading
        /// @param parent state to evaluate
        /// @param depth max shallow depth to evaluate
        /// @param deep_depth max deep depth to evaluate
        /// @param time_limit abort evaluation after the time limit was reached
        /// @return best score that the parent gets
        Result expectiminimax(const State& parent, const uint32_t depth, const uint32_t deep_depth = 0, const double time_limit = parameters::TIME_LIMIT);
    };

    template <typename BaseSearch>
    std::atomic<unsigned int> ThreadedSearch<BaseSearch>::free_threads{1};

    template <typename BaseSearch>
    std::vector<typename ThreadedSearch<BaseSearch>::Result> ThreadedSearch<BaseSearch>::expectiminimaxThreaded(const std::vector<std::unique_ptr<State>>& children, const uint32_t depth, const uint32_t deep_depth, const double time_limit) {
        const std::size_t number_of_children = children.size();

        std::vector<std::future<Result>> futures;
        std::vector<Result> results(number_of_children);
        std::size_t next_future_index = 0;
        std::vector<bool> result_ready(number_of_children, false);
        auto start = std::chrono::steady_clock::now();
        // wait short for quickly finished results
        while (true) {
            // add as many futures as possible
            unsigned int expected = free_threads.load();
            while(free_threads.compare_exchange_weak(expected, expected - 1) && (next_future_index < number_of_children)) {
                auto& child = children[next_future_index];
                futures.push_back(std::async(std::launch::async, [this ,&child, depth, deep_depth]() {
                    Result result;
                    {
                        ExtendedSearch<BaseSearch> single_thread_search;
                        result = single_thread_search.expectiminimax(*child, depth - 1, deep_depth);
                        // scope results in a clearing of memory when this is finished
                    }
                    return result;
                }));
                ++next_future_index;
                expected = free_threads.load();
            }

            std::this_thread::sleep_for(std::chrono::milliseconds(10)); // short timeout if search is quick

            // check future status
            for (std::size_t index = 0; index < number_of_children; ++index) {
                if(index >= next_future_index) break;
                if(result_ready[index]) continue;
                auto& future = futures[index];
                if (future.wait_for(std::chrono::seconds(0)) == std::future_status::ready) {
                    result_ready[index] = true;
                    results[index] = future.get();
                    free_threads.store(free_threads + 1);
                    continue;
                }
                break;
            }

            if(std::all_of(result_ready.begin(), result_ready.end(), [](const bool value) { return value; }))
                break;

            // check timeout
            auto now = std::chrono::steady_clock::now();
            auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(now - start).count();
            if (elapsed >= time_limit) {
                std::cout << "Aborting search after execution time of " << elapsed <<" seconds surpassed the time limit. Waiting for result.\n";
                BaseSearch::timeout.store(true);
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(1000)); // Check periodically
        }

        return results;
    }

    template <typename BaseSearch>
    typename ThreadedSearch<BaseSearch>::Result ThreadedSearch<BaseSearch>::expectiminimaxSingleLayer(const State& parent, const std::vector<std::unique_ptr<State>>& children, const std::vector<Result>& results) {
        Result best_result{};
        const bool is_evaluation = StateMachine::isEvaluationPhase(parent.next_event);
        if(is_evaluation) {
            if (StateMachine::isPlayerTurn(parent)) {
                best_result.score = std::numeric_limits<int>::min();
                for (size_t i = 0; i < results.size(); ++i) {
                    const Result& result = results[i];
                    if (result.score > best_result.score) {
                        best_result = result;
                        best_result.follow_ups.push_front(children[i]->next_event);
                    }
                }
            } else {
                best_result.score = std::numeric_limits<int>::max();
                for (size_t i = 0; i < results.size(); ++i) {
                    const Result& result = results[i];
                    if (result.score < best_result.score) {
                        best_result = result;
                        best_result.follow_ups.push_front(children[i]->next_event);
                    }
                }
            }
        } else {
            // random event happens
            double total_probability = 0.0;
            for (size_t i = 0; i < results.size(); ++i) {
                const Result& result = results[i];
                const auto& child = children[i];
                total_probability += child->probability;
                best_result.score += child->probability * result.score;
            }
            assert(std::abs(total_probability - 1.0) < parameters::EPSILON);
        }

        return best_result;
    }

    template <typename BaseSearch>
    typename ThreadedSearch<BaseSearch>::Result ThreadedSearch<BaseSearch>::expectiminimax(const State& parent, const uint32_t depth, const uint32_t deep_depth, const double time_limit) {
        if (StateMachine::isFinished(parent) || !depth) {
            return ExtendedSearch<BaseSearch>::expectiminimax(parent, depth, deep_depth);
        }

        auto children = StateMachine::getChildStates(parent);
        const auto results = expectiminimaxThreaded(children, depth, deep_depth, time_limit);
        return expectiminimaxSingleLayer(parent, children, results);
    }
}
