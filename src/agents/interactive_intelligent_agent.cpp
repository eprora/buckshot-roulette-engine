#include "engine/agents/interactive_intelligent_agent.hpp"

namespace engine{

    State InteractiveIntelligentAgent::getSuccessor(State state, std::vector<std::unique_ptr<State>> children) {
        const auto rounds_left = state.shotgun.getRemainingRounds();
        if(rounds_left) std::cout << "Probability for blank round: " << StateMachine::getProbabilityOfBlankRound(state, children.front()->inverter_used) << ".\n";

        Search::Result best_choice;
        if(last_result.follow_ups.empty() || last_result.follow_ups.front().is_player_turn != state.next_event.is_player_turn) {
            std::cout << "Evaluating options... (can take a while on the first rounds)\n";
            // evaluate best choice
            best_choice = IntelligentAgent::getBestChoice(state, true);
            std::cout << search::toString<Search::Result, Evaluator>(best_choice);
        } else {
            std::cout << "Using result from previous search:\n";
            best_choice = last_result;
            std::cout << search::toString<Search::Result, Evaluator>(best_choice);
        }
        auto choice = InteractiveAgent::getSuccessor(std::move(state), std::move(children));
        if(choice.next_event == best_choice.follow_ups.front()) {
            best_choice.follow_ups.pop_front();
            last_result = best_choice;
        } else {
            reset();
        }
        return choice;
    }
}