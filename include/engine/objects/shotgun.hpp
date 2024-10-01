#pragma once

#include <deque>
#include <stdexcept>
#include <algorithm>
#include "engine/objects/magazine.hpp"
#include "engine/objects/participant.hpp"

namespace engine{

    class Shotgun : public Magazine{
    public:
        void load(const unsigned int live_rounds, const unsigned int blank_rounds);

        void shootBlankRound();
        void shootLiveRound(Participant& victim);
        void sawOff();
        bool isSawedOff() const;

        std::pair<std::bitset<32>, uint32_t> getHash(const int max_shots) const;

        bool operator==(const Shotgun& other) const;

    private:
        bool sawed_off{false};
    };
}