#pragma once

#include "engine/objects/state.hpp"
#include <memory>

namespace engine{

    class StateMachine {
    public:
        using State = engine::State;
        using Event = engine::Event;

        /// @brief Checks whether a node is a terminal node
        /// @param state State to evaluate
        /// @return True if finished
        static bool isFinished(const State& state);

        /// @brief Checks whether a evalutation phase is happening
        /// @param event follow up event
        /// @return True if evalutation is ongoing, false for random or certain events
        static bool isEvaluationPhase(const Event& event);

        /// @brief Checks whether player is to evaluate options
        /// @param state State to evaluate
        /// @return True if PlayerTurn
        static inline bool isPlayerTurn(const State& state) {
            return state.next_event.is_player_turn;
        }

        /// @brief Computes maximum tree depth until terminal node must be reached
        /// @param state State to evaluate
        /// @return Max depth until a finished state must be reached
        static unsigned int getMaxDepth(const State& state);

        /// @brief Creates child states for a given state
        /// @param parent Parent state
        /// @return List of child states with probabilities
        static std::vector<std::unique_ptr<State>> getChildStates(const State& parent);

        /// @brief Computes the probability of a blank round appearing
        /// @param state State to evaluate
        /// @param inverter_used whether inverter has been used
        /// @param index Index of the Round. 0 is the next round in the chamber
        /// @return Probability of round being blank between 0.0 and 1.0
        static double getProbabilityOfBlankRound(const State& state, const bool inverter_used, const unsigned int index = 0);

    private:
        static std::vector<std::unique_ptr<State>> getEvaluatingChildStates(const State& parent, const bool use_opponent_items = false);
        static std::vector<std::unique_ptr<State>> getPlayerEvaluatingChildStates(const State& parent, const bool use_opponent_items = false);
        static std::vector<std::unique_ptr<State>> getDealerEvaluatingChildStates(const State& parent, const bool use_opponent_items = false);
        static std::vector<std::unique_ptr<State>> getShootSelfChildStates(const State& parent);
        static std::vector<std::unique_ptr<State>> getShootOtherChildStates(const State& parent);
        static std::vector<std::unique_ptr<State>> getUseItemChildStates(const State& parent);
        static std::vector<std::unique_ptr<State>> getGainedRoundKnowledgeChildStates(const State& parent, const unsigned int index);
        static std::unique_ptr<State> getCigaretteChildState(const State& parent);
        static std::vector<std::unique_ptr<State>> getGlassChildStates(const State& parent);
        static std::unique_ptr<State> getSawChildState(const State& parent);
        static std::unique_ptr<State> getHandcuffsChildState(const State& parent);
        static std::vector<std::unique_ptr<State>> getPhoneChildStates(const State& parent);
        static std::vector<std::unique_ptr<State>> getBeerChildStates(const State& parent);
        static std::vector<std::unique_ptr<State>> getPillsChildStates(const State& parent);
        static std::unique_ptr<State> getInverterChildState(const State& parent);
        static std::vector<std::unique_ptr<State>> getInverterChildStates(const State& parent);
        static std::vector<std::unique_ptr<State>> getAdrenalinChildStates(const State& parent);
    };
}
