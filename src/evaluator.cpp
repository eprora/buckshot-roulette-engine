#include "engine/evaluator.hpp"
#include <string>
#include <cassert>

namespace engine{

    double Evaluator::getScore(const State& state){
        if(!state.player.lives) return LOSS_SCORE;
        double player_advantage{0.0};
        if(!state.dealer.lives) player_advantage = WIN_SCORE;
        // heuristic value depends mostly on lives (1 life == 1.0 advantage)
        // only add life advantage if stage is not finished
        else player_advantage = static_cast<double>(state.player.lives) - static_cast<double>(state.dealer.lives);
        // add items
        assert(state.player.items.size() <= game_parameters::MAX_SLOTS);
        assert(state.dealer.items.size() <= game_parameters::MAX_SLOTS);
        const int empty_slots = std::max(state.dealer.items.size(), MAX_SCORING_EMPTY_SLOTS) - std::max(state.player.items.size(), MAX_SCORING_EMPTY_SLOTS);
        player_advantage += static_cast<double>(empty_slots) * SCORES[0];
        for(const auto item : state.player.items) {
            assert(static_cast<unsigned>(item) < SCORES.size());
            player_advantage += SCORES[static_cast<size_t>(item)];
        }
        for(const auto item : state.dealer.items) {
            assert(static_cast<unsigned>(item) < SCORES.size());
            player_advantage -= SCORES[static_cast<size_t>(item)];
        }
        return player_advantage;
    }

    double Evaluator::getWinProbability(const double score){
        return std::clamp((score - LOSS_SCORE)/(WIN_SCORE - LOSS_SCORE), 0.0, 1.0);
    }

    double Evaluator::getScore(const double win_probability){
        return win_probability * WIN_SCORE + (1.0-win_probability) * LOSS_SCORE;
    }
}