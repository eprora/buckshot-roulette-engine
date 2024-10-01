#include "engine/game.hpp"
#include <iostream>
#include "engine/agents/randomized_agent.hpp"
#include "engine/agents/automatic_intelligent_agent.hpp"
#include "engine/item_drawers/randomized_item_drawer.hpp"
#include "randomizer.hpp"
#include <future>
#include <atomic>

namespace {
    std::pair<int, int> playGames(unsigned int num_games_to_play, unsigned int seed) {
        std::unique_ptr<randomizer::TrueRandomizer<engine::State>> randomizer = std::make_unique<randomizer::TrueRandomizer<engine::State>>();
        std::unique_ptr<engine::AutomaticIntelligentAgent> player = std::make_unique<engine::AutomaticIntelligentAgent>();
        std::unique_ptr<engine::RandomizedAgent> dealer = std::make_unique<engine::RandomizedAgent>();
        dealer->setSeed(seed);
        std::unique_ptr<engine::RandomizedItemDrawer> item_drawer = std::make_unique<engine::RandomizedItemDrawer>();
        
        int wins = 0;
        int losses = 0;
        engine::Game game(std::move(randomizer), std::move(player), std::move(dealer), std::move(item_drawer), seed);
        
        while (num_games_to_play > 0) {
            game.startRandomized();
            while (!game.isFinished()) {
                game.playMove();
            }
            if (game.isWon()) {
                ++wins;
                --num_games_to_play;
                std::cout << "Game was won. " << num_games_to_play << " left in thread of seed " << seed << ".\n";
            } else if (game.isLost()) {
                ++losses;
                --num_games_to_play;
                std::cout << "Game was lost. " << num_games_to_play << " left in thread of seed " << seed << ".\n";
            }
        }

        std::cout << "Thread finished.\n";
        return {wins, losses};
    }
}

// ------------------------------MAIN-------------------------------------------------
int main(int argc, char* argv[]){				// argc = number of arguments, argv = array of argument c-strings (argv[1] is the first)

    unsigned int num_games_to_play{1};
    unsigned int num_threads{1};
    std::random_device rd;
    unsigned int seed = rd();
    if(argc > 1) {
        num_games_to_play = strtoul(argv[1], &argv[1], 10);
    }
    if(argc > 2) {
        num_threads = strtoul(argv[2], &argv[2], 10);
    }
    if(argc > 3) {
        seed = strtoul(argv[3], &argv[3], 10);
    }
    std::cout << "seed used: " << seed << "\n";

    int total_wins = 0;
    int total_losses = 0;
    auto start = std::chrono::high_resolution_clock::now();

    if(false /*num_threads > 1*/) {
        // TODO: fix multithreading
        // 1. atomic free_threads must not be a static variable
        // 2. fix deadlock situation with the timeout
        engine::AutomaticIntelligentAgent::Search::free_threads.store(num_threads);
        std::vector<std::future<std::pair<int, int>>> futures;
        for (int i = 0; i < num_threads; ++i) {
            futures.push_back(std::async(std::launch::async, playGames, num_games_to_play, seed + i));
        }
        for (auto& future : futures) {
            auto result = future.get();
            total_wins += result.first;
            total_losses += result.second;
        }
    } else {
        engine::AutomaticIntelligentAgent::Search::free_threads.store(num_threads);
        std::tie(total_wins, total_losses) = playGames(num_games_to_play, seed);
    }
    auto end = std::chrono::high_resolution_clock::now();
    const std::chrono::duration<double> elapsed = end - start;
    std::cout << "Time of execution: " << elapsed.count() << " seconds." << std::endl;
    std::cout << "Execution time per game: " << elapsed.count()/static_cast<double>(num_games_to_play*num_threads) << " seconds." << std::endl;
    std::cout << "Total Wins: " << total_wins << "\n";
    std::cout << "Total Losses: " << total_losses << "\n";
    std::cout << "Win probability: " << static_cast<double>(100*total_wins)/static_cast<double>(total_losses + total_wins) << " %\n";

	return 0;
}