#include "engine/state_machine.hpp"
#include "engine/game_parameters.hpp"
#include "parameters.hpp"
#include <string>
#include <cassert>

namespace engine{
    bool StateMachine::isFinished(const State& state){
        return !state.player.lives || !state.dealer.lives || !state.shotgun.getRemainingRounds();
    }

    bool StateMachine::isEvaluationPhase(const Event& event){
        if(event.action == Action::Evaluating) return true;
        if(event.action != Action::UseItem) return false;
        switch(event.item) {
            case Item::Adrenalin:
            case Item::Cigarette:
            case Item::Saw:
            case Item::Handcuffs:
            case Item::Inverter:
                return true;
            default:
                return false;
        }
    }

    unsigned int StateMachine::getMaxDepth(const State& state){
        // Each evaluation round is followed by use of an item or shotgun shot
        // Therefore maximum tree depth is...
        const auto max_depth = 2 * (state.dealer.items.size() + state.player.items.size() + state.shotgun.getRemainingRounds());
        assert(max_depth <= 48);
        return max_depth;
    }

    std::vector<std::unique_ptr<State>> StateMachine::getChildStates(const State& parent){
        assert(!isFinished(parent));
        // decide by choice
        switch (parent.next_event.action) {
            case Action::Evaluating:
                return getEvaluatingChildStates(parent);
            case Action::ShootSelf:
                return getShootSelfChildStates(parent);
            case Action::ShootOther:
                return getShootOtherChildStates(parent);
            case Action::UseItem:
                return getUseItemChildStates(parent);
            default:
                throw std::runtime_error("Unknown action type in getChildStates");
        }
    }

    std::vector<std::unique_ptr<State>> StateMachine::getEvaluatingChildStates(const State& parent, const bool use_opponent_items){
        if constexpr (parameters::DEALER_USES_PLAYER_LOGIC) {
            return getPlayerEvaluatingChildStates(parent, use_opponent_items);
        }

        if(isPlayerTurn(parent)) {
            return getPlayerEvaluatingChildStates(parent, use_opponent_items);
        } else {
            return getDealerEvaluatingChildStates(parent, use_opponent_items);
        }
    }

    std::vector<std::unique_ptr<State>> StateMachine::getPlayerEvaluatingChildStates(const State& parent, const bool use_opponent_items){

        std::array<bool, game_parameters::ITEMS + 1> items_not_to_consider{};
        const bool is_last_round = parent.shotgun.getRemainingRounds() < 2;
        auto& participant = parent.getActiveParticipant();
        auto& opponent = parent.getOpponent();

        Round known_shell = parent.shotgun.getPlayerKnowledgeOfRound();
        if(parent.inverter_used) {
            if(known_shell == Round::BlankRound) known_shell = Round::LiveRound;
            else if(known_shell == Round::LiveRound) known_shell = Round::BlankRound;
        }
        bool dont_shoot_self = parent.shotgun.isSawedOff() || ((known_shell == Round::LiveRound) && (participant.lives < 2));

        // add children
        std::vector<std::unique_ptr<State>> children{};

        // add items
        for (const auto item : (use_opponent_items ? opponent.items : participant.items)){
            // only consider the items that the player currently owns
            assert(item != Item::None);
            assert(item != Item::Count);
            // don't add already used items
            if(items_not_to_consider[static_cast<unsigned int>(item)]) continue;
            items_not_to_consider[static_cast<unsigned int>(item)] = true;

            // apply some simplification rules
            switch(item) {
                case Item::Glass:
                    if(known_shell != Round::Unknown) continue;
                    break;
                case Item::Saw:
                    if(opponent.lives < 2 || parent.shotgun.isSawedOff()) continue;
                    break;
                case Item::Handcuffs:
                    if(!parent.handcuffs.isAllowedToAdd() || is_last_round) continue;
                    break;
                case Item::Phone:
                    if(is_last_round) continue;
                    break;
                case Item::Beer:
                    if(is_last_round) continue;
                    break;
                case Item::Inverter:
                    // double inverter use leads to a transposition
                    // if the next round is uncertain two inverters will result
                    // in a weird scenario where the next round is "guaranteed"
                    // to be live when in fact it is not.
                    if(parent.inverter_used) continue;
                    break;
                case Item::Adrenalin:
                    if(use_opponent_items || getAdrenalinChildStates(parent).empty()) continue;
                    break;
                case Item::Pills:
                default:
                    break;
            }

            auto child = std::make_unique<State>(parent);
            child->next_event.item = item;
            child->next_event.action = Action::UseItem;
            if(use_opponent_items) {
                child->getOpponent().removeItem(item);
            } else {
                child->getActiveParticipant().removeItem(item);
            }
            children.push_back(std::move(child));
        }

        // adrenalin use forbids shooting options
        if(use_opponent_items) return children; // adrenalin checks if this is empty

        // shoot self or opponent
        if(!dont_shoot_self){
            auto child = std::make_unique<State>(parent);
            child->next_event.action = Action::ShootSelf;
            children.push_back(std::move(child));
        }
        if(!use_opponent_items){
            auto child = std::make_unique<State>(parent);
            child->next_event.action = Action::ShootOther;
            children.push_back(std::move(child));
        }

        assert(!children.empty());
        return children;
    }

    std::vector<std::unique_ptr<State>> StateMachine::getDealerEvaluatingChildStates(const State& parent, const bool use_opponent_items){
        assert(!isPlayerTurn(parent));
        std::array<bool, game_parameters::ITEMS + 1> items_not_to_consider{};
        const bool is_last_round = parent.shotgun.getRemainingRounds() < 2;
        const bool has_max_health = parent.dealer.lives == parent.max_lives;

        bool consider_shooting_self{!parent.shotgun.isSawedOff()}, consider_shooting_other{true};
        Round known_shell = parent.shotgun.getDealerKnowledgeOfRound();
        if(parent.inverter_used) {
            if(known_shell == Round::BlankRound) known_shell = Round::LiveRound;
            else if(known_shell == Round::LiveRound) known_shell = Round::BlankRound;
        }
        // if the dealer uses the phone / glass but the player can't see the result
        bool may_know_round = parent.shotgun.couldDealerKnowRound();
        if(known_shell == Round::LiveRound) {
            consider_shooting_self = false;
        } else if(known_shell == Round::BlankRound) {
            consider_shooting_other = false;
        } else if(!may_know_round) {
            // implement the original "CoinFlip()"" function which
            // is not a coin flip for endless mode
            const unsigned int blank_count = parent.shotgun.getRemainingBlankRounds();
            const unsigned int live_count = parent.shotgun.getRemainingLiveRounds();
            if(blank_count > live_count) consider_shooting_other = false;
            if(blank_count < live_count) consider_shooting_self = false;
            // else is actual coin flip
        }

        // add children
        std::vector<std::unique_ptr<State>> children{};

        // check for saws and cigarettes
        const auto& items = use_opponent_items ? parent.player.items : parent.dealer.items;
        bool has_cigs = std::find(items.begin(), items.end(), Item::Cigarette) != items.end();
        bool has_saw_to_use{false};

        // add items
        for (const auto& item : items){
            // only consider the items that the player currently owns
            assert(item != Item::None);
            assert(item != Item::Count);
            // don't add already used items
            if(items_not_to_consider[static_cast<unsigned int>(item)]) continue;
            items_not_to_consider[static_cast<unsigned int>(item)] = true;

            // apply some simplification rules
            switch(item) {
                case Item::Glass:
                    if(known_shell == Round::Unknown) break;
                    continue;
                case Item::Cigarette:
                    if(!has_max_health) break;
                    continue;
                case Item::Pills:
                    if(!has_max_health && !has_cigs && (parent.dealer.lives != 1)) break;
                    continue;
                case Item::Beer:
                    if(known_shell != Round::LiveRound && !is_last_round) break;
                    continue;
                case Item::Handcuffs:
                    if(parent.handcuffs.isAllowedToAdd() && !is_last_round) break;
                    continue;
                case Item::Saw:
                    // (known_shell == Round::LiveRound || may_know_round) are covered by consider_shooting_other
                    if(consider_shooting_other && !parent.shotgun.isSawedOff()) {
                        has_saw_to_use = true;
                        consider_shooting_other = false;
                        break;
                    }
                    continue;
                case Item::Phone:
                    if(parent.shotgun.getRemainingRounds() > 2) break;
                    continue;
                case Item::Inverter:
                    if((known_shell == Round::BlankRound || may_know_round) && !parent.inverter_used) break;
                    continue;
                case Item::Adrenalin:
                    if(use_opponent_items || getAdrenalinChildStates(parent).empty()) continue;
                    break;
                default:
                    break;
            }

            auto child = std::make_unique<State>(parent);
            child->next_event.item = item;
            child->next_event.action = Action::UseItem;
            if(use_opponent_items) {
                child->getOpponent().removeItem(item);
            } else {
                child->getActiveParticipant().removeItem(item);
            }
            children.push_back(std::move(child));
        }

        // adrenalin use forbids shooting options
        if(use_opponent_items) return children; // adrenalin checks if this is empty

        assert(consider_shooting_other || consider_shooting_self || !children.empty());
        // dealer shoots only if no more usable items exist or might know the round
        if(children.empty() || has_saw_to_use || may_know_round) {
            if(consider_shooting_self){
                auto child = std::make_unique<State>(parent);
                child->next_event.action = Action::ShootSelf;
                children.push_back(std::move(child));
            }
            // dealer will saw before shooting when possible
            if(consider_shooting_other){
                auto child = std::make_unique<State>(parent);
                child->next_event.action = Action::ShootOther;
                children.push_back(std::move(child));
            }
            assert(!children.empty());
        }

        assert(!children.empty());
        return children;
    }

    std::vector<std::unique_ptr<State>> StateMachine::getShootSelfChildStates(const State& parent){
        std::vector<std::unique_ptr<State>> children{};

        double probability = parent.shotgun.getProbabilityOfBlankRound();
        if(std::abs(probability) > parameters::EPSILON) {
            // blank round
            auto child = std::make_unique<State>(parent);
            child->probability = probability;
            if(parent.inverter_used) {
                child->shotgun.convertBlankRound();
                child->shotgun.shootLiveRound(child->getActiveParticipant());
                child->switchParticipantIfNotCuffed();
                child->inverter_used = false;
            } else {
                child->shotgun.shootBlankRound();
            }
            child->next_event.action = Action::Evaluating;
            children.push_back(std::move(child));
        }
        probability = 1.0 - probability;
        if(std::abs(probability) > parameters::EPSILON) {
            // live round
            auto child = std::make_unique<State>(parent);
            child->probability = probability;
            if(parent.inverter_used) {
                child->shotgun.convertLiveRound();
                child->shotgun.shootBlankRound();
                child->inverter_used = false;
            } else {
                child->shotgun.shootLiveRound(child->getActiveParticipant());
                child->switchParticipantIfNotCuffed();
            }
            child->next_event.action = Action::Evaluating;
            children.push_back(std::move(child));
        }
        assert(!children.empty());
        return children;
    }

    std::vector<std::unique_ptr<State>> StateMachine::getShootOtherChildStates(const State& parent){
        std::vector<std::unique_ptr<State>> children{};

        double probability = parent.shotgun.getProbabilityOfBlankRound();
        if(std::abs(probability) > parameters::EPSILON) {
            // blank round
            auto child = std::make_unique<State>(parent);
            child->probability = probability;
            if(parent.inverter_used) {
                child->shotgun.convertBlankRound();
                child->shotgun.shootLiveRound(child->getOpponent());
                child->inverter_used = false;
            } else {
                child->shotgun.shootBlankRound();
            }
            child->switchParticipantIfNotCuffed();
            child->next_event.action = Action::Evaluating;
            children.push_back(std::move(child));
        }
        probability = 1.0 - probability;
        if(std::abs(probability) > parameters::EPSILON) {
            // live round
            auto child = std::make_unique<State>(parent);
            child->probability = probability;
            if(parent.inverter_used) {
                child->shotgun.convertLiveRound();
                child->shotgun.shootBlankRound();
                child->inverter_used = false;
            } else {
                child->shotgun.shootLiveRound(child->getOpponent());
            }
            child->switchParticipantIfNotCuffed();
            child->next_event.action = Action::Evaluating;
            children.push_back(std::move(child));
        }
        assert(!children.empty());
        return children;
    }

    std::vector<std::unique_ptr<State>> StateMachine::getUseItemChildStates(const State& parent){
        // decide by choice
        switch (parent.next_event.item) {
            case Item::Cigarette:
                return getEvaluatingChildStates(*getCigaretteChildState(parent));
            case Item::Glass:
                return getGlassChildStates(parent);
            case Item::Saw:
                return getEvaluatingChildStates(*getSawChildState(parent));
            case Item::Handcuffs:
                return getEvaluatingChildStates(*getHandcuffsChildState(parent));
            case Item::Phone:
                return getPhoneChildStates(parent);
            case Item::Beer:
                return getBeerChildStates(parent);
            case Item::Pills:
                return getPillsChildStates(parent);
            case Item::Inverter:
                return getEvaluatingChildStates(*getInverterChildState(parent));
            case Item::Adrenalin:
                return getAdrenalinChildStates(parent);
            default:
                throw std::runtime_error("Unknown item type when trying to use an item");
        }
    }

    std::vector<std::unique_ptr<State>> StateMachine::getGainedRoundKnowledgeChildStates(const State& parent, const unsigned int index){
        std::vector<std::unique_ptr<State>> children{};

        const bool is_player_turn = isPlayerTurn(parent);

        double probability = parent.shotgun.getProbabilityOfBlankRound(index);
        if(std::abs(probability) > parameters::EPSILON) {
            // blank round
            auto child = std::make_unique<State>(parent);
            child->probability = probability;
            if(!index && parent.inverter_used) {
                child->shotgun.convertBlankRound();
                child->inverter_used = false;
            } else {
                child->shotgun.setBlankRound(index);   
            }
            is_player_turn ? child->shotgun.makePlayerKnowRound(index) : child->shotgun.makeDealerKnowRound(index);
            child->next_event.action = Action::Evaluating;
            children.push_back(std::move(child));
        }
        probability = 1.0 - probability;
        if(std::abs(probability) > parameters::EPSILON) {
            // live round
            auto child = std::make_unique<State>(parent);
            child->probability = probability;
            if(!index && parent.inverter_used) {
                child->shotgun.convertLiveRound();
                child->inverter_used = false;
            } else {
                child->shotgun.setLiveRound(index);
            }
            is_player_turn ? child->shotgun.makePlayerKnowRound(index) : child->shotgun.makeDealerKnowRound(index);
            child->next_event.action = Action::Evaluating;
            children.push_back(std::move(child));
        }
        assert(!children.empty());
        return children;
    }

    std::unique_ptr<State> StateMachine::getCigaretteChildState(const State& parent){
        auto child = std::make_unique<State>(parent);
        child->getActiveParticipant().gainLives(1, parent.max_lives);
        child->next_event.action = Action::Evaluating;
        return std::move(child);
    }

    std::vector<std::unique_ptr<State>> StateMachine::getGlassChildStates(const State& parent){
        return getGainedRoundKnowledgeChildStates(parent, 0);
    }

   std::unique_ptr<State> StateMachine::getSawChildState(const State& parent){
        auto child = std::make_unique<State>(parent);
        child->shotgun.sawOff();
        child->next_event.action = Action::Evaluating;
        return std::move(child);
    }

    std::unique_ptr<State> StateMachine::getHandcuffsChildState(const State& parent){
        auto child = std::make_unique<State>(parent);
        child->handcuffs.add();
        child->next_event.action = Action::Evaluating;
        return std::move(child);
    }

    std::vector<std::unique_ptr<State>> StateMachine::getPhoneChildStates(const State& parent){
        std::vector<std::unique_ptr<State>> children{};
        const auto left_rounds = parent.shotgun.getRemainingRounds();

        // phone is useless for one round left
        if(left_rounds < 2) {
            auto child = std::make_unique<State>(parent);
            child->next_event.action = Action::Evaluating;
            children.push_back(std::move(child));
            assert(!children.empty());
            return children;
        }

        for(unsigned int index = 1; index < left_rounds; ++index) {
            auto new_children = getGainedRoundKnowledgeChildStates(parent, index);
            for(auto& child : new_children) {
                child->probability *= 1.0/static_cast<double>(left_rounds - 1);
            }
            children.insert(children.end(), std::make_move_iterator(new_children.begin()), std::make_move_iterator(new_children.end()));
        }

        assert(!children.empty());
        return children;
    }

    std::vector<std::unique_ptr<State>> StateMachine::getBeerChildStates(const State& parent){
        std::vector<std::unique_ptr<State>> children{};

        double probability = parent.shotgun.getProbabilityOfBlankRound();
        if(std::abs(probability) > parameters::EPSILON) {
            // blank round
            auto child = std::make_unique<State>(parent);
            child->probability = probability;
            if(parent.inverter_used) {
                child->shotgun.convertBlankRound();
                child->shotgun.ejectLiveRound();
                child->inverter_used = false;
            } else {
                child->shotgun.ejectBlankRound();
            }
            child->next_event.action = Action::Evaluating;
            children.push_back(std::move(child));
        }
        probability = 1.0 - probability;
        if(std::abs(probability) > parameters::EPSILON) {
            // live round
            auto child = std::make_unique<State>(parent);
            child->probability = probability;
            if(parent.inverter_used) {
                child->shotgun.convertLiveRound();
                child->shotgun.ejectBlankRound();
                child->inverter_used = false;
            } else {
                child->shotgun.ejectLiveRound();
            }
            child->next_event.action = Action::Evaluating;
            children.push_back(std::move(child));
        }
        assert(!children.empty());
        return children;
    }

    std::vector<std::unique_ptr<State>> StateMachine::getPillsChildStates(const State& parent){
        std::vector<std::unique_ptr<State>> children{};
        {
            // 50% chance to lose 1 health (unless max health is already reached)
            auto child = std::make_unique<State>(parent);
            child->probability = 0.5;
            child->getActiveParticipant().loseLife();
            child->next_event.action = Action::Evaluating;
            children.push_back(std::move(child));
        }
        {
            // 50% chance to gain 2 health (unless max health is already reached)
            auto child = std::make_unique<State>(parent);
            child->probability = 0.5;
            child->getActiveParticipant().gainLives(2, parent.max_lives);
            child->next_event.action = Action::Evaluating;
            children.push_back(std::move(child));
        }
        assert(!children.empty());
        return children;
    }

    std::unique_ptr<State> StateMachine::getInverterChildState(const State& parent){
        auto child = std::make_unique<State>(parent);
        child->inverter_used = true;
        child->next_event.action = Action::Evaluating;
        return std::move(child);
    }

    std::vector<std::unique_ptr<State>> StateMachine::getAdrenalinChildStates(const State& parent) {
        return getEvaluatingChildStates(parent, true);
    }

    double StateMachine::getProbabilityOfBlankRound(const State& state, const bool inverter_used, const unsigned int index) {
        if(index == 0 && inverter_used) return 1.0 - state.shotgun.getProbabilityOfBlankRound(index);
        return state.shotgun.getProbabilityOfBlankRound(index);
    }
}