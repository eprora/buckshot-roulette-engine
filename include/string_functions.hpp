#pragma once

#include <string>
#include "engine/objects/types.hpp"
#include "engine/state_machine.hpp"

namespace engine{
    std::string toString(const engine::Action action);

    std::string toString(const engine::Item item);

    std::string toString(const engine::Event& event);

    void visualizeTreeHelper(const engine::State& node, unsigned int depth, std::string& result);

    std::string visualizeTree(const engine::State& root);
}

namespace search {
    template <class ResultType, class Evaluator>
    std::string toString(const ResultType& event, const bool convert = true) {
        std::string result;
        if(!event.follow_ups.empty()) {
            for(const auto& follow_up : event.follow_ups) {
                result += toString(follow_up) + ". ";
            }
        }
        if(convert) result += "Score: " + std::to_string(Evaluator::getWinProbability(event.score)) + "\n";
        else result += "Score: " + std::to_string(event.score) + "\n";
        return result;
    }
}