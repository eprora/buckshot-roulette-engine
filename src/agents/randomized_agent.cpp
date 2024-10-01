#include "engine/agents/randomized_agent.hpp"
#include "string_functions.hpp"
#include <limits>

namespace engine{

    engine::State RandomizedAgent::getSuccessor(State state, std::vector<std::unique_ptr<engine::State>> children) {
        std::uniform_int_distribution<> option_distribution(0, children.size() - 1);
        return *children[option_distribution(random_number_generator)];
    }
}