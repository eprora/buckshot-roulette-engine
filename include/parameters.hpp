#pragma once

namespace parameters{

    // probabilities smaller than this are considered as impossible 
    constexpr double EPSILON{1.e-10};

    // use this as the maximum shallow depth
    static constexpr unsigned int MAX_SHALLOW_DEPTH{3};
    
    // default time limit
    static constexpr double TIME_LIMIT{30.0};

    // by default, the dealer will use the ingame logic
    static constexpr bool DEALER_USES_PLAYER_LOGIC{false};
}