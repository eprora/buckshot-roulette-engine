#include "engine/objects/magazine.hpp"
#include "parameters.hpp"
#include <cassert>

namespace engine{

    void Magazine::load(const unsigned int live_rounds, const unsigned int blank_rounds) {
        total_live_rounds = unknown_live_rounds = live_rounds;
        total_blank_rounds = unknown_blank_rounds = blank_rounds;
        round_knowledge.clear();
        for(unsigned int idx = 0; idx < live_rounds + blank_rounds; ++idx) {
            round_knowledge.push_back({Round::Unknown, false});
        }
    }
    
    void Magazine::makeDealerKnowRound(const unsigned int index) {
        assert(index < round_knowledge.size());
        round_knowledge[index].dealer_knowledge = true;
    }
    
    void Magazine::makeDealerPossiblyKnowRound(const unsigned int index) {
        assert(index < round_knowledge.size());
        round_knowledge[index].possible_dealer_knowledge = true;
    }
    
    void Magazine::makePlayerKnowRound(const unsigned int index) {
        assert(index < round_knowledge.size());
        round_knowledge[index].player_knowledge = true;
    }

    void Magazine::setBlankRound(const unsigned int index) {
        assert(index < round_knowledge.size());
        assert(getProbabilityOfBlankRound(index) > parameters::EPSILON);
        assert(round_knowledge[index].true_state != Round::LiveRound);
        if(round_knowledge[index].true_state == Round::Unknown) {
            --unknown_blank_rounds;
            round_knowledge[index].true_state = Round::BlankRound;
        }
    }

    void Magazine::setLiveRound(const unsigned int index) {
        assert(index < round_knowledge.size());
        assert(1.0 - getProbabilityOfBlankRound(index) > parameters::EPSILON);
        assert(round_knowledge[index].true_state != Round::BlankRound);
        if(round_knowledge[index].true_state == Round::Unknown) {
            --unknown_live_rounds;
            round_knowledge[index].true_state = Round::LiveRound;
        }
    }

    void Magazine::ejectBlankRound() {
        assert(!round_knowledge.empty());
        assert(total_blank_rounds);
        assert(getProbabilityOfBlankRound() > parameters::EPSILON);
        if(round_knowledge.front().true_state == Round::Unknown) {
            assert(unknown_blank_rounds);
            --unknown_blank_rounds;
        }
        --total_blank_rounds;
        round_knowledge.pop_front();
    }

    void Magazine::ejectLiveRound() {
        assert(!round_knowledge.empty());
        assert(total_live_rounds);
        assert(1.0 - getProbabilityOfBlankRound() > parameters::EPSILON);
        if(round_knowledge.front().true_state == Round::Unknown) {
            assert(unknown_live_rounds);
            --unknown_live_rounds;
        }
        --total_live_rounds;
        round_knowledge.pop_front();
    }

    void Magazine::convertBlankRound() {
        assert(!round_knowledge.empty());
        assert(total_blank_rounds);
        assert(getProbabilityOfBlankRound() > parameters::EPSILON);
        if(round_knowledge.front().true_state == Round::Unknown) {
            assert(unknown_blank_rounds);
            --unknown_blank_rounds;
        }
        --total_blank_rounds;
        ++total_live_rounds;
        round_knowledge.front().true_state = Round::LiveRound;
    }

    void Magazine::convertLiveRound() {
        assert(!round_knowledge.empty());
        assert(total_live_rounds);
        assert(1.0 - getProbabilityOfBlankRound() > parameters::EPSILON);
        if(round_knowledge.front().true_state == Round::Unknown) {
            assert(unknown_live_rounds);
            --unknown_live_rounds;
        }
        --total_live_rounds;
        ++total_blank_rounds;
        round_knowledge.front().true_state = Round::BlankRound;
    }
    
    bool Magazine::couldDealerKnowRound(const unsigned int index) const{
        return round_knowledge[index].possible_dealer_knowledge || (round_knowledge[index].dealer_knowledge && (round_knowledge[index].true_state == Round::Unknown));
    }
    
    Round Magazine::getDealerKnowledgeOfRound(const unsigned int index) const{
        assert(index < round_knowledge.size());
        if(round_knowledge[index].dealer_knowledge) return round_knowledge[index].true_state;
        unsigned int dealer_unknown_blank_rounds = unknown_blank_rounds;
        unsigned int dealer_unknown_live_rounds = unknown_live_rounds;
        for(unsigned int idx = 0; idx < round_knowledge.size(); ++idx) {
            if(!round_knowledge[idx].dealer_knowledge) {
                if(round_knowledge[idx].true_state == Round::BlankRound) ++dealer_unknown_blank_rounds;
                if(round_knowledge[idx].true_state == Round::LiveRound) ++dealer_unknown_live_rounds;
            }
        }
        if(!dealer_unknown_live_rounds) return Round::BlankRound;
        if(!dealer_unknown_blank_rounds) return Round::LiveRound;
        return Round::Unknown;
    }
    
    Round Magazine::getPlayerKnowledgeOfRound(const unsigned int index) const{
        assert(index < round_knowledge.size());
        if(round_knowledge[index].player_knowledge) return round_knowledge[index].true_state;
        unsigned int player_unknown_blank_rounds = unknown_blank_rounds;
        unsigned int player_unknown_live_rounds = unknown_live_rounds;
        for(unsigned int idx = 0; idx < round_knowledge.size(); ++idx) {
            if(!round_knowledge[idx].player_knowledge) {
                if(round_knowledge[idx].true_state == Round::BlankRound) ++player_unknown_blank_rounds;
                if(round_knowledge[idx].true_state == Round::LiveRound) ++player_unknown_live_rounds;
            }
        }
        if(!player_unknown_live_rounds) return Round::BlankRound;
        if(!player_unknown_blank_rounds) return Round::LiveRound;
        return Round::Unknown;
    }

    double Magazine::getProbabilityOfBlankRound(const unsigned int index) const{
        assert(!round_knowledge.empty());
        assert(index < round_knowledge.size());
        // consider if the first round is known
        if(round_knowledge[index].true_state == Round::BlankRound) return 1.0;
        if(round_knowledge[index].true_state == Round::LiveRound) return 0.0;
        return unknown_blank_rounds / static_cast<double>(unknown_live_rounds + unknown_blank_rounds);
    }
}