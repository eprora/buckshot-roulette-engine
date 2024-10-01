#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_session.hpp>
#include "engine/state_machine.hpp"
#include "engine/evaluator.hpp"
#include "search/threaded_search.hpp"
#include "string_functions.hpp"
#include <iostream>
#include <chrono>
#include <bitset>

TEST_CASE("Participant hash test", "[Participant]") {
    engine::Participant participant;
    participant.lives = 3;

    auto [hash, length] = participant.getHash(8);

    REQUIRE(hash.to_ulong() == 0b110000);
    REQUIRE(length == 6);
}

TEST_CASE("Event hash test", "[Event]") {
    engine::Event event;

    auto [hash, length] = event.getHash();

    REQUIRE(hash.to_ulong() == 0b11);
    REQUIRE(length == 2);
}

TEST_CASE("Handcuffs hash test", "[Handcuffs]") {
    engine::Handcuffs handcuffs;
    handcuffs.add();

    auto [hash, length] = handcuffs.getHash();

    REQUIRE(hash.to_ulong() == 0b1);
    REQUIRE(length == 1);
}

TEST_CASE("Participant hash test", "[Shotgun]") {
    engine::Shotgun shotgun;
    shotgun.sawOff();

    auto [hash, length] = shotgun.getHash(8);

    REQUIRE(hash.to_ulong() == 0b100000000);
    REQUIRE(length == 9);
}

int main(int argc, char* argv[]) {
    Catch::Session session;

    int result = session.applyCommandLine(argc, argv);
    if (result != 0) {
        return result;
    }
    return session.run();
}