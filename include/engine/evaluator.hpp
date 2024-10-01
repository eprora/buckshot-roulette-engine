#pragma once

#include "engine/objects/state.hpp"
#include "engine/game_parameters.hpp"

namespace engine{
    class SimpleEvaluator{
    public:
        /// @brief Returns a heuristic score for the state
        /// @param state State to evaluate
        /// @return Score
        static double getScore(const State& state){
            if(!state.player.lives) return 0.0;
            if(!state.dealer.lives) return 1.0;
            return 0.5;
        }

        /// @brief Converts the score value to win probability
        /// @param score 
        /// @return probability in [0.0, 1.0]
        static double getWinProbability(const double score){
            return score;
        }
    };

    class Evaluator{
    public:
        /// @brief Returns a heuristic score for the state
        /// @param state State to evaluate
        /// @return Value in range (0.0, 1.0) is non-winning state. <=0.0 indicates losing. >= 1.0 indicates winning.
        static double getScore(const State& state);

        /// @brief Converts the score value to win probability
        /// @param score 
        /// @return probability in [0.0, 1.0]
        static double getWinProbability(const double score);

        /// @brief Converts the win_probability value to score
        /// @param win_probability 
        /// @return score in [LOSS_SCORE, WIN_SCORE]
        static double getScore(const double win_probability);

    private:
        // see getExpectedAdvantage function for reasoning
        // normal chances are: 50% chance of 1 damage
        // None: estimated value
        // Cigarette: guaranteed +1 health
        // Glass: 50% guaranteed shot, 50% 50:50
        // Saw: 50% chance of double damage
        // Handcuffs: double 50% chance of 1 damage
        // Pills: 50% of -1 health, 50% of +2 health -> 50% of Cigarette score
        // Adrenalin: estimated value
        static constexpr std::array<double, 10> SCORES{0.015, 0.1, 0.025, 0.05, 0.05, 0.02, 0.01, 0.05, 0.01, 0.075};

        // max lives for max lives
        static constexpr double MAX_ADVANTAGE{static_cast<double>(1 + game_parameters::MAX_LIVES)};
        static constexpr std::size_t MAX_SCORING_EMPTY_SLOTS{game_parameters::MAX_SLOTS - game_parameters::MAX_ITEM_DRAW};
        static constexpr double LOSS_SCORE{-MAX_ADVANTAGE*9};
        static constexpr double WIN_SCORE{MAX_ADVANTAGE};

        // estimate for the probability of an empty slot to be replaced by a drawn item
        // this is not how it works but assuming this simplifies the evaluation functions A LOT!
        static constexpr double ITEM_DRAW_PROBABILITY_FOR_EMPTY_SLOT{0.5};
    };
}