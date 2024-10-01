#include "engine/agents/automatic_intelligent_agent.hpp"

namespace engine{

    State AutomaticIntelligentAgent::getSuccessor(State state, std::vector<std::unique_ptr<State>> children) {
        if(logging) std::cout << "Evaluating options... (can take a while on the first rounds)\n";
        Search::Result best_choice;
        if(last_result.follow_ups.empty() || last_result.follow_ups.front().is_player_turn != state.next_event.is_player_turn) {
            best_choice = getBestChoice(state, logging);
        } else {
            best_choice = last_result;
        }
        // find best choice in children
        for(auto& child : children){
            if(child->next_event == best_choice.follow_ups.front()) {
                if(logging) std::cout << search::toString<Search::Result, Evaluator>(best_choice);
                best_choice.follow_ups.pop_front();
                last_result = best_choice;
                return std::move(*child);
            }
        }
        last_result = {};
        throw std::runtime_error("Could not find best choice in available options.");
    }
}