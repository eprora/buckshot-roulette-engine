#include <catch2/catch_template_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>
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

using Evaluator = engine::Evaluator;
using StateMachine = engine::StateMachine;
using Solver = search::ThreadedSearch<search::TranspositionSearch<StateMachine, Evaluator>>;

#define SOLVER_TYPES (search::ThreadedSearch<search::TranspositionSearch<StateMachine, Evaluator>>), (search::ExtendedSearch<search::TranspositionSearch<StateMachine, Evaluator>>), (search::ThreadedSearch<search::Search<StateMachine, Evaluator>>), (search::ExtendedSearch<search::Search<StateMachine, Evaluator>>), (search::ExtendedSearch<search::IterativeSearch<search::TranspositionSearch<StateMachine, Evaluator>>>)
#define DEBUG_TYPE (search::ExtendedSearch<search::Search<StateMachine, Evaluator>>)

namespace {
    constexpr unsigned int max_shallow_depth = parameters::MAX_SHALLOW_DEPTH;
    constexpr unsigned int max_deep_depth = 48; // maximum possible
}

template <typename Solver>
struct TestFixture {
    Solver solver;
    engine::State start_state{};
    using Result = typename Solver::Result;
    Result expected_result{};

    void swap() {
        // swaps player and dealer
        std::swap(start_state.player, start_state.dealer);
        start_state.next_event.is_player_turn = !start_state.next_event.is_player_turn;
        for(auto& event : expected_result.follow_ups) {
            event.is_player_turn = !event.is_player_turn;
        }
        expected_result.score = 1.0 - expected_result.score;
    }

    void run() {
        bool do_swap = GENERATE(false, true);
        if(do_swap) {
            swap();
        }
        runSingleScenario();
    }

    void visualizeTree() {
        std::cout << engine::visualizeTree(start_state);
    }

    void runPlayerScenario() {
        runSingleScenario();
    }

    void runDealerScenario() {
        swap();
        runSingleScenario();
    }

    void runSingleScenario() {
        auto result = solver.expectiminimax(start_state, max_shallow_depth, max_deep_depth);
        result.score = Evaluator::getWinProbability(result.score);
        expected_result.score = Evaluator::getWinProbability(Evaluator::getScore(expected_result.score));

        if (result != expected_result) {
            std::cout << "Search result: " << search::toString<Result, Evaluator>(result, false);
            std::cout << "Expected: " << search::toString<Result, Evaluator>(expected_result, false);
        }

        REQUIRE(result == expected_result);
        this->expected_result.follow_ups.clear();
    }
};

/// REMINDER
/// ALL DRAWS RESULT IN SCORE 0.5

TEMPLATE_TEST_CASE_METHOD(TestFixture, "Participant dead", "[search][template][player][dealer]", SOLVER_TYPES) {
    this->start_state.shotgun.load(0, 0);
    this->start_state.resetLives(1);
    this->start_state.player.loseLife();
    this->expected_result.score = 0.0;
    this->run();
}

TEMPLATE_TEST_CASE_METHOD(TestFixture, "Opponent dead", "[search][template][player][dealer]", SOLVER_TYPES) {
    this->start_state.shotgun.load(0, 0);
    this->start_state.resetLives(1);
    this->start_state.dealer.loseLife();
    this->expected_result.score = 1.0;
    this->run();
}

TEMPLATE_TEST_CASE_METHOD(TestFixture, "Finished game with equal chance", "[search][template][player]", SOLVER_TYPES) {
    this->start_state.shotgun.load(0, 0);
    this->start_state.resetLives(1);
    // score 0.0 = equal field
    this->expected_result.score = Evaluator::getWinProbability(0.0);
    this->runPlayerScenario();
}

TEMPLATE_TEST_CASE_METHOD(TestFixture, "Finished game with equal chance", "[search][template][dealer]", SOLVER_TYPES) {
    this->start_state.shotgun.load(0, 0);
    this->start_state.resetLives(1);
    // score 0.0 = equal field
    this->expected_result.score = 1.0 - Evaluator::getWinProbability(0.0);
    this->runDealerScenario();
}

TEMPLATE_TEST_CASE_METHOD(TestFixture, "ShootOther case", "[search][template][player][dealer]", SOLVER_TYPES) {
    this->start_state.shotgun.load(1,0);
    this->start_state.resetLives(1);
    this->expected_result.score = 1.0;
    this->expected_result.follow_ups.push_back({true, engine::Action::ShootOther});
    this->run();
}

TEMPLATE_TEST_CASE_METHOD(TestFixture, "Saw case", "[search][template][player][dealer]", SOLVER_TYPES) {
    engine::Item special_item = engine::Item::Saw;
    this->start_state.shotgun.load(1,0);
    this->start_state.resetLives(2);
    this->start_state.player.items = {special_item};
    this->expected_result.score = 1.0;
    this->expected_result.follow_ups.push_back({true, engine::Action::UseItem, special_item});
    this->expected_result.follow_ups.push_back({true, engine::Action::ShootOther});
    this->run();
}

TEMPLATE_TEST_CASE_METHOD(TestFixture, "Inverter case", "[search][template][player][dealer]", SOLVER_TYPES) {
    engine::Item special_item = engine::Item::Inverter;
    this->start_state.shotgun.load(0,1);
    this->start_state.resetLives(1);
    this->start_state.player.items = {special_item};
    this->expected_result.score = 1.0;
    this->expected_result.follow_ups.push_back({true, engine::Action::UseItem, special_item});
    this->expected_result.follow_ups.push_back({true, engine::Action::ShootOther});
    this->run();
}

TEMPLATE_TEST_CASE_METHOD(TestFixture, "Inverter spare case", "[search][template][player][dealer][spare]", SOLVER_TYPES) {
    engine::Item special_item = engine::Item::Inverter;
    this->start_state.shotgun.load(2,0);
    this->start_state.resetLives(1);
    this->expected_result.score = 1.0;
    this->start_state.player.items = {special_item, special_item};
    this->start_state.dealer.items = {special_item, special_item};
    this->expected_result.follow_ups.push_back({true, engine::Action::ShootOther});
    this->run();
}

TEMPLATE_TEST_CASE_METHOD(TestFixture, "Inverter case unclear", "[search][template][player]", SOLVER_TYPES) {
    engine::Item special_item = engine::Item::Inverter;
    this->start_state.shotgun.load(1,1);
    this->start_state.resetLives(1);
    this->start_state.player.items = {special_item};
    this->expected_result.score = 0.5*(1 + Evaluator::getWinProbability(0.0)); // 50% win
    this->expected_result.follow_ups.push_back({true, engine::Action::UseItem, special_item});
    this->expected_result.follow_ups.push_back({true, engine::Action::ShootOther});
    // dealer won't use inverter if he is not sure about it being blank.
    this->runPlayerScenario();
}

TEMPLATE_TEST_CASE_METHOD(TestFixture, "Inverter + glass case", "[search][template][player]", SOLVER_TYPES) {
    this->start_state.shotgun.load(1,1);
    this->start_state.resetLives(1);
    // in the code using inverter will set the true state of first round.
    // this will result in 50% chance of live round first
    // and 50% chance of blank round converted to live round with inverter
    // and the algorithm will suggest inverter has 100 % win chance which is not the case
    // glass is actually the best solution her
    this->start_state.player.items = {engine::Item::Inverter, engine::Item::Inverter, engine::Item::Glass};
    this->expected_result.score = 1.0;
    this->expected_result.follow_ups.push_back({true, engine::Action::UseItem, engine::Item::Glass});
    this->runPlayerScenario();
}

TEMPLATE_TEST_CASE_METHOD(TestFixture, "Inverter + glass case", "[search][template][dealer]", SOLVER_TYPES) {
    this->start_state.shotgun.load(1,1);
    this->start_state.resetLives(2);
    this->start_state.player.items = {engine::Item::Inverter, engine::Item::Glass};
    this->start_state.dealer.items = {engine::Item::Inverter, engine::Item::Saw};
    this->expected_result.score = 0.0;
    // dealer will use all available items so glass first
    // if blank -> inverter will be used. Will shoot opponent. Second shell is live.
    // Opponent has saw and will one-hit kill dealer.
    // if live -> will shoot opponent. Since opponent has inverter, second shell can be converted.
    // Again one hit kill of dealer.
    this->expected_result.follow_ups.push_back({true, engine::Action::UseItem, engine::Item::Glass});
    this->runDealerScenario();
}

TEMPLATE_TEST_CASE_METHOD(TestFixture, "Handcuffs case", "[search][template][player][dealer]", SOLVER_TYPES) {
    engine::Item special_item = engine::Item::Handcuffs;
    this->start_state.shotgun.load(1,1);
    this->start_state.resetLives(1);
    this->start_state.player.items = {special_item};
    this->expected_result.score = 1.0;
    this->expected_result.follow_ups.push_back({true, engine::Action::UseItem, special_item});
    this->expected_result.follow_ups.push_back({true, engine::Action::ShootOther});
    this->run();
}

TEMPLATE_TEST_CASE_METHOD(TestFixture, "Glass case", "[search][template][player][dealer]", SOLVER_TYPES) {
    engine::Item special_item = engine::Item::Glass;
    this->start_state.shotgun.load(1,1);
    this->start_state.resetLives(1);
    this->start_state.player.items = {special_item};
    this->expected_result.score = 1.0;
    this->expected_result.follow_ups.push_back({true, engine::Action::UseItem, special_item});
    this->run();
}

TEMPLATE_TEST_CASE_METHOD(TestFixture, "Phone case", "[search][template][player]", SOLVER_TYPES) {
    engine::Item special_item = engine::Item::Phone;
    this->start_state.shotgun.load(1,1);
    this->start_state.resetLives(1);
    this->start_state.player.items = {special_item};
    this->expected_result.score = 1.0;
    this->expected_result.follow_ups.push_back({true, engine::Action::UseItem, special_item});
    this->runPlayerScenario();
}

TEMPLATE_TEST_CASE_METHOD(TestFixture, "Phone case", "[search][template][dealer]", SOLVER_TYPES) {
    engine::Item special_item = engine::Item::Phone;
    this->start_state.shotgun.load(2,2);
    this->start_state.resetLives(1);
    this->start_state.player.items = {special_item};
    // dealer won't use phone at 2 items or less
    this->expected_result.score = 2.0/3.0;
    this->expected_result.follow_ups.push_back({true, engine::Action::UseItem, special_item});
    this->runDealerScenario();
}

TEMPLATE_TEST_CASE_METHOD(TestFixture, "Phone spare case", "[search][template][player][dealer][spare]", SOLVER_TYPES) {
    engine::Item special_item = engine::Item::Phone;
    this->start_state.shotgun.load(2,0);
    this->start_state.resetLives(1);
    this->expected_result.score = 1.0;
    this->start_state.player.items = {special_item};
    this->start_state.dealer.items = {special_item};
    this->expected_result.follow_ups.push_back({true, engine::Action::ShootOther});
    this->run();
}

TEMPLATE_TEST_CASE_METHOD(TestFixture, "Adrenalin case", "[search][template][player][dealer]", SOLVER_TYPES) {
    this->start_state.shotgun.load(1,1);
    this->start_state.resetLives(1);
    this->start_state.player.items = {engine::Item::Adrenalin};
    this->start_state.dealer.items = {engine::Item::Handcuffs};
    this->expected_result.score = 1.0;
    this->expected_result.follow_ups.push_back({true, engine::Action::UseItem, engine::Item::Adrenalin});
    this->expected_result.follow_ups.push_back({true, engine::Action::UseItem, engine::Item::Handcuffs});
    this->expected_result.follow_ups.push_back({true, engine::Action::ShootOther});
    this->run();
}

TEMPLATE_TEST_CASE_METHOD(TestFixture, "Beer case", "[search][template][player][dealer]", SOLVER_TYPES) {
    this->start_state.shotgun.load(2,1);
    this->start_state.shotgun.setLiveRound(1);
    this->start_state.resetLives(1);
    this->expected_result.score = 1.0;
    this->start_state.player.items = {engine::Item::Beer};
    this->expected_result.follow_ups.push_back({true, engine::Action::UseItem, engine::Item::Beer});
    this->run();
}

TEMPLATE_TEST_CASE_METHOD(TestFixture, "Beer spare case", "[search][template][player][spare]", SOLVER_TYPES) {
    engine::Item special_item = engine::Item::Beer;
    this->start_state.shotgun.load(2,0);
    this->start_state.resetLives(1);
    this->expected_result.score = 1.0;
    this->start_state.player.items = {special_item};
    this->start_state.dealer.items = {special_item};
    this->expected_result.follow_ups.push_back({true, engine::Action::ShootOther});
    this->runPlayerScenario();
}

TEMPLATE_TEST_CASE_METHOD(TestFixture, "Beer use up case", "[search][template][player][spare]", SOLVER_TYPES) {
    engine::Item special_item = engine::Item::Beer;
    this->start_state.shotgun.load(3,0);
    this->start_state.resetLives(1);
    this->expected_result.score = 1.0;
    this->start_state.player.items = {special_item, special_item, special_item, special_item, special_item, special_item, special_item, special_item};
    this->expected_result.follow_ups.push_back({true, engine::Action::UseItem, special_item});
    this->expected_result.follow_ups.push_back({true, engine::Action::UseItem, special_item});
    this->expected_result.follow_ups.push_back({true, engine::Action::ShootOther});
    this->runPlayerScenario();
}

TEMPLATE_TEST_CASE_METHOD(TestFixture, "Cigarette case", "[search][template][player][dealer]", SOLVER_TYPES) {
    engine::Item special_item = engine::Item::Cigarette;
    this->start_state.shotgun.load(3,0);
    this->start_state.resetLives(2);
    this->start_state.player.loseLife();
    this->expected_result.score = 1.0;
    this->start_state.player.items = {special_item};
    this->expected_result.follow_ups.push_back({true, engine::Action::UseItem, special_item});
    this->expected_result.follow_ups.push_back({true, engine::Action::ShootOther});
    this->expected_result.follow_ups.push_back({false, engine::Action::ShootOther});
    this->expected_result.follow_ups.push_back({true, engine::Action::ShootOther});
    this->run();
}

TEMPLATE_TEST_CASE_METHOD(TestFixture, "Cigarette spare case", "[search][template][player][dealer][spare]", SOLVER_TYPES) {
    engine::Item special_item = engine::Item::Cigarette;
    this->start_state.shotgun.load(2,0);
    this->start_state.resetLives(1);
    this->expected_result.score = 1.0;
    this->start_state.player.items = {special_item};
    this->start_state.dealer.items = {special_item};
    this->expected_result.follow_ups.push_back({true, engine::Action::ShootOther});
    this->run();
}

TEMPLATE_TEST_CASE_METHOD(TestFixture, "Pills case", "[search][template][player][dealer]", SOLVER_TYPES) {
    engine::Item special_item = engine::Item::Pills;
    this->start_state.shotgun.load(5,0);
    this->start_state.resetLives(3);
    this->start_state.player.loseLife();
    this->expected_result.score = 0.5;
    this->start_state.player.items = {special_item};
    this->expected_result.follow_ups.push_back({true, engine::Action::UseItem, special_item});
    this->runPlayerScenario();
}

TEMPLATE_TEST_CASE_METHOD(TestFixture, "No items, probability only", "[search][template][player]", SOLVER_TYPES) {
    this->start_state.shotgun.load(1,2);
    this->start_state.resetLives(1);
    this->expected_result.score = 2.0/3.0;
    this->expected_result.follow_ups.push_back({true, engine::Action::ShootOther});
    this->runPlayerScenario();
}

TEMPLATE_TEST_CASE_METHOD(TestFixture, "No items, probability only", "[search][template][dealer]", SOLVER_TYPES) {
    this->start_state.shotgun.load(1,2);
    this->start_state.resetLives(1);
    this->expected_result.score = 1.0/3.0;
    // There are more blanks in the gun so dealer will shoot themselves
    this->expected_result.follow_ups.push_back({true, engine::Action::ShootSelf});
    this->runDealerScenario();
}

TEMPLATE_TEST_CASE_METHOD(TestFixture, "Beer beer beer case", "[search][template][player][dealer]", SOLVER_TYPES) {
    // technically x live, 1 blank round are guaranteed win with x beers and 1 inverter 
    this->start_state.shotgun.load(7,1);
    this->start_state.resetLives(1);
    this->expected_result.score = 1.0;
    this->start_state.player.items = {engine::Item::Beer, engine::Item::Beer, engine::Item::Beer, engine::Item::Beer, 
                                engine::Item::Beer, engine::Item::Beer, engine::Item::Beer, engine::Item::Inverter};
    this->expected_result.follow_ups.push_back({true, engine::Action::UseItem, engine::Item::Beer});
    this->run();
}

TEMPLATE_TEST_CASE_METHOD(TestFixture, "4 rounds case", "[search][template][player][dealer]", SOLVER_TYPES) {
    this->start_state.shotgun.load(2,2);
    this->start_state.resetLives(1);
    this->expected_result.score = 1.0;
    // if beer is used you end up with 2 rounds
    // phone can clear up 50:50 scenarios
    // inverter can invert to live round
    // this is how you win guaranteed
    this->start_state.player.items = {engine::Item::Inverter, engine::Item::Phone, engine::Item::Adrenalin, engine::Item::Adrenalin};
    this->start_state.dealer.items = {engine::Item::Beer, engine::Item::Beer};
    this->expected_result.follow_ups.push_back({true, engine::Action::UseItem, engine::Item::Phone});
    this->run();
}

int main(int argc, char* argv[]) {
    Catch::Session session;

    int result = session.applyCommandLine(argc, argv);
    if (result != 0) {
        return result;
    }
    return session.run();
}