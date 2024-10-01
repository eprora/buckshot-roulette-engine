#include <catch2/catch_template_test_macros.hpp>
#include <catch2/catch_session.hpp>
#include "engine/state_machine.hpp"
#include "engine/evaluator.hpp"
#include "search/threaded_search.hpp"
#include "search/iterative_search.hpp"
#include "search/transposition_search.hpp"
#include "search/search.hpp"
#include "string_functions.hpp"
#include <iostream>
#include <chrono>

namespace {
    constexpr unsigned int max_shallow_depth = parameters::MAX_SHALLOW_DEPTH;
    constexpr unsigned int max_deep_depth = 6;
}

template <typename Solver>
void run_test(unsigned int max_shallow_depth, unsigned int max_deep_depth, const engine::State& start_state) {
    Solver solver;
    auto start = std::chrono::high_resolution_clock::now();
    const auto result = solver.expectiminimax(start_state, max_shallow_depth, max_deep_depth);
    auto end = std::chrono::high_resolution_clock::now();
    const std::chrono::duration<double> elapsed = end - start;
    std::cout << "Time of algorithm execution: " << elapsed.count() << " seconds." << std::endl;
}

TEMPLATE_TEST_CASE("Algorithm Performance Test", "[template]", 
                   (search::ThreadedSearch<search::TranspositionSearch<engine::StateMachine, engine::Evaluator>>),
                   (search::ExtendedSearch<search::TranspositionSearch<engine::StateMachine, engine::Evaluator>>),
                   (search::ThreadedSearch<search::Search<engine::StateMachine, engine::Evaluator>>),
                   (search::ExtendedSearch<search::Search<engine::StateMachine, engine::Evaluator>>),
                   (search::ThreadedSearch<search::IterativeSearch<search::TranspositionSearch<engine::StateMachine, engine::Evaluator>>>),
                   (search::ExtendedSearch<search::IterativeSearch<search::TranspositionSearch<engine::StateMachine, engine::Evaluator>>>)) {

    engine::State start_state;
    start_state.shotgun.load(4, 4);
    start_state.resetLives(4);
    start_state.player.items = {engine::Item::Beer, engine::Item::Glass, engine::Item::Phone, engine::Item::Saw,
                                engine::Item::Inverter, engine::Item::Pills, engine::Item::Cigarette, engine::Item::Adrenalin};
    start_state.dealer.items = {engine::Item::Handcuffs, engine::Item::Glass, engine::Item::Phone, engine::Item::Saw,
                                engine::Item::Inverter, engine::Item::Pills, engine::Item::Cigarette, engine::Item::Adrenalin};

    run_test<TestType>(max_shallow_depth, max_deep_depth, start_state);
}

int main(int argc, char* argv[]) {
    Catch::Session session;

    int result = session.applyCommandLine(argc, argv);
    if (result != 0) {
        return result;
    }
    return session.run();
}