#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>
#include <catch2/catch_session.hpp>
#include <deque>

#include "engine/agents/automatic_intelligent_agent.hpp"
#include "engine/game.hpp"
#include "string_functions.hpp"

namespace {
    using State = engine::State;
    using Game = engine::Game;
    using Event = engine::Event;
    using Action = engine::Action;
    using Item = engine::Item;

    class FakeRandomizer : public randomizer::Randomizer<State> {
    public:
        std::deque<unsigned int> choices;

        void setSeed(unsigned int seed) override{ return; };
        State getSuccessor(const std::vector<std::unique_ptr<State>>& children) override{ 
            const auto choice = getChoice();
            std::cout << "Choice is " << choice << ".\n";
            if(choice >= children.size()) {
                FAIL("Could not find choice number.\n");
            }
            return *children.at(choice);
        }
        virtual State getHiddenKnowledgeSuccessor(State state, std::vector<std::unique_ptr<State>> children, const bool is_phone) { 
            return getSuccessor(std::move(children));
        }
    private:
        unsigned int getChoice() {
            if(choices.empty()) FAIL("No more children of fake randomizer specified");
            const auto answer = choices.front();
            choices.pop_front();
            return answer;
        }
    };

    class FakeHiddenRandomizer : public FakeRandomizer, public randomizer::GetInputRandomizer<State> {
        public:
        State getHiddenKnowledgeSuccessor(State state, std::vector<std::unique_ptr<State>> children, const bool is_phone) override{ 
            return randomizer::GetInputRandomizer<State>::getHiddenKnowledgeSuccessor(std::move(state), std::move(children), is_phone);
        }
    };

    class FakeAgent : public engine::Agent {
    public:
        std::deque<Event> choices;

        State getSuccessor(State state, std::vector<std::unique_ptr<State>> children) override{
            if(!choices.empty()) {
                const auto choice = choices.front();
                for(const auto& child : children) {
                    if(child->next_event == choice) {
                        std::cout << "Choice is " << engine::toString(choice) << ".\n";
                        choices.pop_front();
                        return *child;
                    }
                }
            }
            FAIL("Could not find choice: " + engine::toString(choices.front()) + ".\n");
            return *children.front();
        }
        void confirm() const override{ return; }
        void reset() override{ return; }
    };

    class FakeItemDrawer : public engine::ItemDrawer {
    public:
        std::vector<engine::Item> player_items;
        std::vector<engine::Item> dealer_items;
        void setSeed(unsigned int seed) override{ return; };
        std::pair<std::vector<engine::Item>, std::vector<engine::Item>> getItems(const unsigned int max_health, std::vector<engine::Item> player_items, std::vector<engine::Item> dealer_items) override{ return {this->player_items, this->dealer_items}; }
    };

    enum GameResult{ WIN, LOSS, DRAW };
}

struct TestFixture {
    std::unique_ptr<FakeRandomizer> randomizer = std::make_unique<FakeRandomizer>();
    std::unique_ptr<FakeAgent> player = std::make_unique<FakeAgent>();
    std::unique_ptr<FakeAgent> dealer = std::make_unique<FakeAgent>();
    std::unique_ptr<FakeItemDrawer> item_drawer = std::make_unique<FakeItemDrawer>();

    Game getGame() {
        return Game(std::move(randomizer), std::move(player), std::move(dealer), std::move(item_drawer), 0, true);
    }

    void requireResult(Game& game, const GameResult result){
        std::cout << "Game:---------------------------------\n";
        while(!game.isFinished()){
            std::cout << "Move:------------------\n";
            game.playMove();
        }
        switch (result)
        {
        case WIN:
            REQUIRE(game.isWon());
            REQUIRE_FALSE(game.isLost());
            break;
        case LOSS:
            REQUIRE_FALSE(game.isWon());
            REQUIRE(game.isLost());
            break;
        case DRAW:
            REQUIRE_FALSE(game.isWon());
            REQUIRE_FALSE(game.isLost());
            break;
        default:
            break;
        }
    }
};

TEST_CASE_METHOD(TestFixture, "Opponent dead", "[game][player]") {
    player->choices = {{true, Action::ShootOther}};
    auto game = getGame();
    game.start(1,0,1);
    requireResult(game, WIN);
}

TEST_CASE_METHOD(TestFixture, "Nobody dead", "[game][player]") {
    player->choices = {{true, Action::ShootSelf}};
    auto game = getGame();
    game.start(0,1,1);
    requireResult(game, DRAW);
}

TEST_CASE_METHOD(TestFixture, "Participant dead", "[game][player]") {
    player->choices = {{true, Action::ShootOther}};
    dealer->choices = {{false, Action::ShootOther}};
    randomizer->choices = {0};
    auto game = getGame();
    game.start(1,1,1);
    requireResult(game, LOSS);
}

TEST_CASE_METHOD(TestFixture, "Cigarette use", "[game][player]") {
    item_drawer->dealer_items = {engine::Item::Cigarette};
    player->choices = {{true, Action::ShootOther}, {true, Action::ShootOther}};
    dealer->choices = {{false, Action::UseItem, Item::Cigarette}, {false, Action::ShootOther}, {false, Action::ShootOther}};
    auto game = getGame();
    game.start(4,0,2);
    requireResult(game, LOSS);
}

TEST_CASE_METHOD(TestFixture, "Glass use", "[game][player]") {
    item_drawer->player_items = {engine::Item::Glass};
    player->choices = {{true, Action::UseItem, Item::Glass}, {true, Action::ShootSelf}, {true, Action::ShootOther}};
    randomizer->choices = {0};
    auto game = getGame();
    game.start(1,1,1);
    requireResult(game, WIN);
}

TEST_CASE_METHOD(TestFixture, "Saw use", "[game][player]") {
    item_drawer->player_items = {engine::Item::Saw};
    player->choices = {{true, Action::UseItem, Item::Saw}, {true, Action::ShootOther}};
    auto game = getGame();
    game.start(1,0,2);
    requireResult(game, WIN);
}

TEST_CASE_METHOD(TestFixture, "Handcuff use", "[game][player]") {
    item_drawer->player_items = {engine::Item::Handcuffs};
    player->choices = {{true, Action::UseItem, Item::Handcuffs}, {true, Action::ShootOther}, {true, Action::ShootOther}};
    auto game = getGame();
    game.start(2,0,2);
    requireResult(game, WIN);
}

TEST_CASE_METHOD(TestFixture, "Phone use (uncertain outcome)", "[game][player]") {
    item_drawer->player_items = {engine::Item::Phone};
    player->choices = {{true, Action::UseItem, Item::Phone}, {true, Action::ShootSelf}, {true, Action::ShootSelf}, {true, Action::ShootOther}};
    randomizer->choices = {3};
    auto game = getGame();
    game.start(1,2,1);
    requireResult(game, WIN);
}

TEST_CASE_METHOD(TestFixture, "Phone use (certain outcome)", "[game][player]") {
    item_drawer->player_items = {engine::Item::Glass, engine::Item::Phone};
    player->choices = {{true, Action::UseItem, Item::Glass}, {true, Action::UseItem, Item::Phone}, {true, Action::ShootSelf}, {true, Action::ShootOther}};
    randomizer->choices = {0,0};
    auto game = getGame();
    game.start(2,1,1);
    requireResult(game, WIN);
}

TEST_CASE_METHOD(TestFixture, "Beer use (uncertain outcome)", "[game][player]") {
    item_drawer->player_items = {engine::Item::Beer};
    player->choices = {{true, Action::UseItem, Item::Beer}, {true, Action::ShootOther}};
    randomizer->choices = {0};
    auto game = getGame();
    game.start(1,1,1);
    requireResult(game, WIN);
}

TEST_CASE_METHOD(TestFixture, "Beer use (certain outcome)", "[game][player]") {
    item_drawer->player_items = {engine::Item::Glass, engine::Item::Beer};
    player->choices = {{true, Action::UseItem, Item::Glass}, {true, Action::UseItem, Item::Beer}, {true, Action::ShootOther}};
    randomizer->choices = {1};
    auto game = getGame();
    game.start(1,1,1);
    requireResult(game, DRAW);
}

TEST_CASE_METHOD(TestFixture, "Pills use", "[game][player]") {
    item_drawer->player_items = {engine::Item::Pills};
    player->choices = {{true, Action::ShootOther}, {true, Action::UseItem, Item::Pills}};
    dealer->choices = {{false, Action::ShootOther}};
    randomizer->choices = {0,1,0};
    auto game = getGame();
    game.start(2,2,2);
    requireResult(game, LOSS);
}

TEST_CASE_METHOD(TestFixture, "Inverter use (certain outcome)", "[game][player]") {
    item_drawer->player_items = {engine::Item::Inverter};
    player->choices = {{true, Action::UseItem, Item::Inverter}, {true, Action::ShootSelf}, {true, Action::ShootOther}};
    dealer->choices = {{false, Action::ShootOther}}; // as a back up but should not happen in ideal case
    auto game = getGame();
    game.start(2,0,1);
    requireResult(game, WIN);
}

TEST_CASE_METHOD(TestFixture, "Inverter use (uncertain outcome)", "[game][player]") {
    item_drawer->player_items = {engine::Item::Inverter};
    player->choices = {{true, Action::UseItem, Item::Inverter}, {true, Action::ShootOther}};
    dealer->choices = {{false, Action::ShootSelf}};
    randomizer->choices = {0};
    auto game = getGame();
    game.start(1,1,1);
    requireResult(game, DRAW);
}

TEST_CASE_METHOD(TestFixture, "Adrenalin use", "[game][player]") {
    item_drawer->player_items = {engine::Item::Adrenalin};
    item_drawer->dealer_items = {engine::Item::Saw};
    player->choices = {{true, Action::UseItem, Item::Adrenalin}, {true, Action::UseItem, Item::Saw}, {true, Action::ShootOther}};
    auto game = getGame();
    game.start(1,0,2);
    requireResult(game, WIN);
}

TEST_CASE_METHOD(TestFixture, "Inverter + Saw", "[game][player]") {
    // this whould be same for Pills, Cigarette, Handcuffs since they are commutative
    item_drawer->player_items = {engine::Item::Inverter, engine::Item::Adrenalin};
    item_drawer->dealer_items = {engine::Item::Saw};
    player->choices = {{true, Action::UseItem, Item::Inverter}, {true, Action::UseItem, Item::Adrenalin}, {true, Action::UseItem, Item::Saw}, {true, Action::ShootOther}};
    randomizer->choices = {1};
    auto game = getGame();
    game.start(1,1,2);
    requireResult(game, WIN);
}

TEST_CASE_METHOD(TestFixture, "Inverter + Phone", "[game][player]") {
    item_drawer->player_items = {engine::Item::Inverter, engine::Item::Phone};
    player->choices = {{true, Action::UseItem, Item::Inverter}, {true, Action::UseItem, Item::Phone}, {true, Action::ShootOther}};
    randomizer->choices = {1};
    auto game = getGame();
    game.start(1,1,1);
    requireResult(game, WIN);
}

TEST_CASE_METHOD(TestFixture, "Inverter + Glass", "[game][player]") {
    item_drawer->player_items = {engine::Item::Inverter, engine::Item::Glass};
    player->choices = {{true, Action::UseItem, Item::Inverter}, {true, Action::UseItem, Item::Glass}, {true, Action::ShootSelf}, {true, Action::ShootOther}};
    randomizer->choices = {0};
    auto game = getGame();
    game.start(1,1,1);
    requireResult(game, DRAW);
}

TEST_CASE_METHOD(TestFixture, "Inverter + Beer", "[game][player]") {
    item_drawer->player_items = {engine::Item::Inverter, engine::Item::Beer};
    player->choices = {{true, Action::UseItem, Item::Inverter}, {true, Action::UseItem, Item::Beer}, {true, Action::ShootOther}};
    randomizer->choices = {1};
    auto game = getGame();
    game.start(1,1,1);
    requireResult(game, WIN);
}

TEST_CASE_METHOD(TestFixture, "Dealer shoots self without using saw", "[game][dealer]") {
    item_drawer->dealer_items = {engine::Item::Saw};
    player->choices = {{true, Action::ShootOther}};
    dealer->choices = {{false, Action::ShootSelf}};
    randomizer->choices = {0,1};
    auto game = getGame();
    game.start(1,2,1);
    requireResult(game, WIN);
}

TEST_CASE_METHOD(TestFixture, "Dealer shoots player with saw", "[game][dealer]") {
    item_drawer->dealer_items = {engine::Item::Saw};
    player->choices = {{true, Action::ShootOther}};
    dealer->choices = {{false, Action::UseItem, Item::Saw}, {false, Action::ShootOther}};
    randomizer->choices = {0,1};
    auto game = getGame();
    game.start(1,2,2);
    requireResult(game, LOSS);
}

TEST_CASE_METHOD(TestFixture, "Hidden outcome(Glass) + Glass", "[game][dealer][hidden]") {
    randomizer = std::make_unique<FakeHiddenRandomizer>();
    item_drawer->dealer_items = {engine::Item::Glass, engine::Item::Glass};
    player->choices = {{true, Action::ShootOther}};
    // dealer won't use glass twice in a row
    dealer->choices = {{false, Action::UseItem, Item::Glass}, {false, Action::ShootOther}};
    randomizer->choices = {0,1};
    auto game = getGame();
    game.start(1,2,1);
    requireResult(game, LOSS);
}

TEST_CASE_METHOD(TestFixture, "Hidden outcome(Glass) + Inverter", "[game][dealer][hidden]") {
    randomizer = std::make_unique<FakeHiddenRandomizer>();
    item_drawer->dealer_items = {engine::Item::Inverter, engine::Item::Glass};
    player->choices = {{true, Action::ShootOther}};
    dealer->choices = {{false, Action::UseItem, Item::Glass}, {false, Action::UseItem, Item::Inverter}, {false, Action::ShootOther}};
    // the hidden choice is not part of the randomizer choices
    randomizer->choices = {0,1};
    auto game = getGame();
    game.start(1,2,1);
    requireResult(game, LOSS);
}

TEST_CASE_METHOD(TestFixture, "Hidden outcome(Glass) + Inverter + Adrenalin + Saw", "[game][dealer][hidden]") {
    randomizer = std::make_unique<FakeHiddenRandomizer>();
    item_drawer->player_items = {engine::Item::Saw};
    item_drawer->dealer_items = {engine::Item::Inverter, engine::Item::Glass, engine::Item::Adrenalin};
    player->choices = {{true, Action::ShootOther}};
    dealer->choices = {{false, Action::UseItem, Item::Glass}, {false, Action::UseItem, Item::Inverter}, {false, Action::UseItem, Item::Adrenalin}, {false, Action::UseItem, Item::Saw}, {false, Action::ShootOther}};
    // the hidden choice is not part of the randomizer choices
    randomizer->choices = {0,1};
    auto game = getGame();
    game.start(1,2,2);
    requireResult(game, LOSS);
}

TEST_CASE_METHOD(TestFixture, "Hidden outcome(Glass) + Saw", "[game][dealer][hidden]") {
    randomizer = std::make_unique<FakeHiddenRandomizer>();
    item_drawer->dealer_items = {engine::Item::Saw, engine::Item::Glass};
    player->choices = {{true, Action::ShootOther}};
    dealer->choices = {{false, Action::UseItem, Item::Glass}, {false, Action::UseItem, Item::Saw}, {false, Action::ShootOther}};
    // the hidden choice is not part of the randomizer choices
    randomizer->choices = {0,1};
    auto game = getGame();
    game.start(1,2,2);
    requireResult(game, LOSS);
}

TEST_CASE_METHOD(TestFixture, "Hidden outcome(Glass) + Beer", "[game][dealer][hidden]") {
    randomizer = std::make_unique<FakeHiddenRandomizer>();
    item_drawer->dealer_items = {engine::Item::Beer, engine::Item::Glass};
    player->choices = {{true, Action::ShootOther}};
    dealer->choices = {{false, Action::UseItem, Item::Glass}, {false, Action::ShootOther}};
    // if the dealer sees a live round beer use is skipped
    randomizer->choices = {0,1};
    auto game = getGame();
    game.start(1,2,1);
    requireResult(game, LOSS);
}

TEST_CASE_METHOD(TestFixture, "Hidden outcome(Phone) + Inverter", "[game][dealer][hidden]") {
    randomizer = std::make_unique<FakeHiddenRandomizer>();
    item_drawer->dealer_items = {engine::Item::Inverter, engine::Item::Phone};
    player->choices = {{true, Action::ShootOther}};
    dealer->choices = {{false, Action::UseItem, Item::Phone}, {false, Action::ShootSelf}, {false, Action::UseItem, Item::Inverter}, {false, Action::ShootOther}};
    // the hidden choice is not part of the randomizer choices
    randomizer->choices = {0,0,1};
    auto game = getGame();
    game.start(1,3,1);
    requireResult(game, LOSS);
}

TEST_CASE_METHOD(TestFixture, "Hidden outcome(Phone) + Saw", "[game][dealer][hidden]") {
    randomizer = std::make_unique<FakeHiddenRandomizer>();
    item_drawer->dealer_items = {engine::Item::Saw, engine::Item::Phone};
    player->choices = {{true, Action::ShootOther}};
    dealer->choices = {{false, Action::UseItem, Item::Phone}, {false, Action::ShootSelf}, {false, Action::UseItem, Item::Saw}, {false, Action::ShootOther}};
    // the hidden choice is not part of the randomizer choices
    randomizer->choices = {0,0,1};
    auto game = getGame();
    game.start(1,3,2);
    requireResult(game, LOSS);
}

TEST_CASE_METHOD(TestFixture, "Weird forced inverter bug", "[game][dealer][hidden]") {
    randomizer = std::make_unique<FakeHiddenRandomizer>();
    item_drawer->player_items = {engine::Item::Phone};
    item_drawer->dealer_items = {engine::Item::Inverter};
    player->choices = {{true, Action::UseItem, Item::Phone}, {true, Action::ShootOther}};
    dealer->choices = {{false, Action::ShootSelf}, {false, Action::ShootOther}};
    // the hidden choice is not part of the randomizer choices
    randomizer->choices = {3};
    auto game = getGame();
    game.start(1,3,1);
    requireResult(game, LOSS);
}

TEST_CASE_METHOD(TestFixture, "Inverter strategy of Automatic Intelligent Agent", "[game][player][automatic]") {
    item_drawer->player_items = {engine::Item::Handcuffs, engine::Item::Phone, engine::Item::Inverter, engine::Item::Inverter, engine::Item::Adrenalin, engine::Item::Adrenalin};
    item_drawer->dealer_items = {engine::Item::Beer, engine::Item::Beer};
    dealer->choices = {{false, Action::ShootSelf}};
    // the hidden choice is not part of the randomizer choices
    randomizer->choices = {1,0,0};
    auto game = Game(std::move(randomizer), std::make_unique<engine::AutomaticIntelligentAgent>(20.0, true), std::move(dealer), std::move(item_drawer), 0, true);
    game.start(2,2,2);
    requireResult(game, WIN);
}

TEST_CASE_METHOD(TestFixture, "Memory clearing of Automatic Intelligent Agent", "[game][player][automatic]") {
    dealer->choices = {{false, Action::ShootOther}};
    // the hidden choice is not part of the randomizer choices
    auto game = Game(std::move(randomizer), std::make_unique<engine::AutomaticIntelligentAgent>(20.0, true), std::move(dealer), std::move(item_drawer), 0, true);
    game.start(6,0,2);
    requireResult(game, WIN);
}

int main(int argc, char* argv[]) {
    Catch::Session session;

    int result = session.applyCommandLine(argc, argv);
    if (result != 0) {
        return result;
    }
    return session.run();
}