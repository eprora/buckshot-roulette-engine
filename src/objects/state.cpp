#include "engine/objects/state.hpp"

namespace engine{
    void State::switchParticipantIfNotCuffed() {
        handcuffs.decay();
        if(handcuffs.isAllowedToAdd()) {
            next_event.is_player_turn = !next_event.is_player_turn;
        }
    }

    
    void State::resetLives(const unsigned int lives) {
        max_lives = player.lives = dealer.lives = lives;
    }
    
    Participant& State::getActiveParticipant() {
        return next_event.is_player_turn? player : dealer;
    }
    
    const Participant& State::getActiveParticipant() const{
        return next_event.is_player_turn? player : dealer;
    }
    
    Participant& State::getOpponent() {
        return next_event.is_player_turn? dealer : player;
    }
    
    const Participant& State::getOpponent() const{
        return next_event.is_player_turn? dealer : player;
    }

    bool State::operator==(const State& other) const {
        return (dealer == other.dealer) && 
        (player == other.player) &&
        (max_lives == other.max_lives) &&
        (shotgun == other.shotgun) &&
        (handcuffs == other.handcuffs) &&
        (next_event == other.next_event);
    }
}