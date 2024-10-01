#pragma once

#include <deque>
#include <stdexcept>
#include "engine/objects/types.hpp"

namespace engine{

    class Magazine{
    public:
        void load(const unsigned int live_rounds, const unsigned int blank_rounds);

        void makeDealerKnowRound(const unsigned int index);
        void makeDealerPossiblyKnowRound(const unsigned int index);
        void makePlayerKnowRound(const unsigned int index);
        void setBlankRound(const unsigned int index);
        void setLiveRound(const unsigned int index);
        void ejectBlankRound();
        void ejectLiveRound();
        void convertBlankRound();
        void convertLiveRound();
        
        unsigned int getRemainingRounds() const{
            return round_knowledge.size();
        }
        unsigned int getRemainingBlankRounds() const{
            return total_blank_rounds;
        }
        unsigned int getRemainingLiveRounds() const{
            return total_live_rounds;
        }
        bool couldDealerKnowRound(const unsigned int index = 0) const;
        Round getDealerKnowledgeOfRound(const unsigned int index = 0) const;
        Round getPlayerKnowledgeOfRound(const unsigned int index = 0) const;
        double getProbabilityOfBlankRound(const unsigned int index = 0) const;

        unsigned int unknown_live_rounds{};
        unsigned int unknown_blank_rounds{};
        unsigned int total_live_rounds{};
        unsigned int total_blank_rounds{};
        std::deque<RoundKnowledge> round_knowledge{};
    };
}