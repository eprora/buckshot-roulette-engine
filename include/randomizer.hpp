#pragma once
#include <cassert>
#include <random>
#include <iostream>

namespace randomizer{
    // Base class Randomizer
    template<typename State>
    class Randomizer {
    public:
        virtual ~Randomizer() = default;
        virtual void setSeed(unsigned int seed) = 0;
        virtual State getSuccessor(const std::vector<std::unique_ptr<State>>& children) = 0;
        virtual State getHiddenKnowledgeSuccessor(State state, std::vector<std::unique_ptr<State>> children, const bool is_phone) = 0;

        bool logging = false;
    };

    // Derived class TrueRandomizer
    template<typename State>
    class TrueRandomizer : public Randomizer<State> {
    private:
        std::mt19937 random_number_generator;
        
        unsigned int selectRandomState(const std::vector<std::unique_ptr<State>>& states) {
            // Create a vector of cumulative probabilities
            std::vector<double> cumulative_probabilities(states.size());
            cumulative_probabilities.front() = states.front()->probability;
            for(unsigned int i = 1; i < states.size(); ++i) {
                cumulative_probabilities[i] = cumulative_probabilities[i-1] + states[i]->probability;
            }

            // Generate a random number in the range [0, total probability)
            std::uniform_real_distribution<> distribution(0.0, cumulative_probabilities.back());
            double randomValue = distribution(random_number_generator);

            // Find the state corresponding to the random value
            auto it = std::lower_bound(cumulative_probabilities.begin(), cumulative_probabilities.end(), randomValue);
            return std::distance(cumulative_probabilities.begin(), it);
        }

    public:
        TrueRandomizer(const bool activate_logging = false) {this->logging = activate_logging;}
        void setSeed(unsigned int seed) override{ random_number_generator = std::mt19937(seed); };

        State getSuccessor(const std::vector<std::unique_ptr<State>>& children) override {
            const unsigned int chosen_option = selectRandomState(children);
            if(this->logging) std::cout << "Option " << chosen_option << " was chosen randomly.\n";
            assert(chosen_option < children.size());
            return *children[chosen_option];
        }

        State getHiddenKnowledgeSuccessor(State state, std::vector<std::unique_ptr<State>> children, const bool is_phone) override {
            const unsigned int chosen_option = selectRandomState(children);
            if(this->logging) std::cout << "An option was chosen randomly.\n";
            assert(chosen_option < children.size());
            return *children[chosen_option];
        }
    };

    // Derived class GetInputRandomizer
    template<typename State>
    class GetInputRandomizer : public Randomizer<State> {
    public:
        GetInputRandomizer(const bool activate_logging = false) {this->logging = activate_logging;}
        void setSeed(unsigned int seed) override{ return; };

        State getSuccessor(const std::vector<std::unique_ptr<State>>& children) override { 
            if(this->logging) std::cout << "Enter the chosen option: \n";
            unsigned int option;
            do {
                std::cin >> option;
                std::cin.sync();
                std::cin.clear();
            } while(option >= children.size());
            return *children[option];
        }

        State getHiddenKnowledgeSuccessor(State state, std::vector<std::unique_ptr<State>> children, const bool is_phone) override {
            if(this->logging) std::cout << "The outcome is unclear and the knowledge is hidden.\n";
            state.next_event = children.front()->next_event;
            if(!is_phone) state.shotgun.makeDealerKnowRound(0);
            else {
                for(int index = 1; index < state.shotgun.getRemainingRounds(); ++index) {
                    state.shotgun.makeDealerPossiblyKnowRound(index);
                }
            }
            return state;
        }
    };
}