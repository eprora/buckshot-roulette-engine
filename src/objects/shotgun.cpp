#include "engine/objects/shotgun.hpp"

namespace engine{
    void Shotgun::load(const unsigned int live_rounds, const unsigned int blank_rounds){
        sawed_off = false;
        Magazine::load(live_rounds, blank_rounds);
    }

    void Shotgun::shootBlankRound() {
        sawed_off = false;
        ejectBlankRound();
    }

    void Shotgun::shootLiveRound(Participant& victim) {
        victim.loseLife();
        if(sawed_off) victim.loseLife();
        sawed_off = false;
        ejectLiveRound();
    }

    void Shotgun::sawOff(){
        if(!sawed_off) {
            sawed_off = true;
            return;
        }
        throw std::logic_error("saw is already in use and can't be used again");
    }

    bool Shotgun::isSawedOff() const{
        return sawed_off;
    }

    std::pair<std::bitset<32>, uint32_t> Shotgun::getHash(const int max_shots) const{

        // leading two bits for the lives
        std::bitset<32> hash;
        for(int i = 0; i < round_knowledge.size(); ++i){
            if(round_knowledge[i].getHash()) hash.set(i);
        }
        if(sawed_off) hash.set(max_shots);

        return {hash, max_shots + 1};
    }

    bool Shotgun::operator==(const Shotgun& other) const {
        if(unknown_blank_rounds != other.unknown_blank_rounds) return false;
        if(unknown_live_rounds != other.unknown_live_rounds) return false;
        if(round_knowledge.size() != other.round_knowledge.size()) return false;
        for(int i = 0; i < round_knowledge.size(); ++i){
            if(round_knowledge[i] != other.round_knowledge[i]) return false;
        }
        return sawed_off == other.isSawedOff();
    }
}