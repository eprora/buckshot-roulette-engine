#pragma once

#include <array>
#include <vector>
#include <deque>
#include <tuple>
#include <stdexcept>
#include "engine/objects/participant.hpp"
#include "engine/objects/shotgun.hpp"
#include "engine/objects/handcuffs.hpp"

namespace engine{

    class State{
    public:
        double probability{1.0};
        Participant player{};
        Participant dealer{};
        Shotgun shotgun{};
        Handcuffs handcuffs{};
        bool inverter_used{false};
        Event next_event{};
        
        // handcuffs cannot be used twice in a row
        unsigned int max_lives{};

        // apply some event
        void switchParticipantIfNotCuffed();
        void resetLives(const unsigned int lives);

        Participant& getActiveParticipant();
        const Participant& getActiveParticipant() const;
        Participant& getOpponent();
        const Participant& getOpponent() const;

        bool operator==(const State& other) const;
    };
}

namespace std{
    template <>
    struct hash<engine::State> {
        std::size_t operator()(const engine::State& state) const {
            const auto [h1, l1] = state.player.getHash(8); // 19
            const auto [h2, l2] = state.dealer.getHash(8);
            const auto [h3, l3] = state.shotgun.getHash(8); // 9
            const auto [h4, l4] = state.handcuffs.getHash(); // 1
            const auto [h5, l5] = state.next_event.getHash(); // 2

            std::size_t hash = h1.to_ulong() ^ h2.to_ulong();
            hash = hash << l3 | h3.to_ulong();
            hash = hash << l4 | h4.to_ulong();
            hash = hash << l5 | h5.to_ulong();
            hash = hash << 1 | (state.max_lives % 2);
            hash = hash << 1 | state.inverter_used;
            return hash;
        }
    };
}