# Buckshot Roulette Engine

[![License](https://img.shields.io/badge/License-Boost_1.0-lightblue.svg)](https://www.boost.org/LICENSE_1_0.txt)

This project implements a search for optimal moves in the computer game "Buckshot Roulette" by Mike Klubnika.

## Features

* Efficient use-case-independant Expectiminimax search algorithm to find the optimal move utilizing optimization techniques.
* The ability to follow games based on observation (alongside playing the game) and suggest moves.
* The ability to simulate games with randomized lives, shotgun shells and items according to the game rules.
* The ability to play against the search algorithm.
* Dealer choice and game randomization is based on game AI.
* Ability to set a deterministic seed.
* Game scenarios can be automatized in code.
* Includes special items added to later versions of Buckshot Roulette (like Adrenalin)

## How to use

The executable `BRengine.exe` starts a console application. The first questions are there to configure the randomization and player/dealer strategy and are answered by entering `+` or `-` and then pressing `ENTER`. Similarly, following questions are answered by entering a number and then pressing `ENTER`. Items need to be entered one by one. Confirmation is also done by just pressing `ENTER`. The application runs indefinitely until the window is closed or `CTRL` + `C` is invoked.

The executable `BRsimulation.exe` runs a benchmark test of the current implemented algorithm against a dealer with randomized strategy. It can be provided with three arguments: The number of games, the number of parallel threads and the seed in this order. The default will be one game, one thread and a random seed. At the end the number of wins, losses and the execution time is shown. 

## How to build and test

This is a CMake project. The only dependency is `Catch2` for the tests and it is imported via the CMake configuration with a fixed version. The tests run in CTest. The project was successfully build using `GCC 12.2.0 x86_64-w64-mingw32`.

The code uses assertions which are not present in the Release build. Therefore it is possible that the Release build might reach illegal states (it is not stable). To debug the code use a Debug build which will make you aware if and where in the code that happens.

## Game Rules

Buckshot Roulette is essentially a variant of the game Russian Roulette but played with a shotgun. A shotgun is loaded with a known amount of live and blank rounds in an unknown order. The player and dealer each take turns in shooting the gun. Each participant can point the shotgun either towards themselves or their opponent. Shooting yourself with a blank round round skips the turn of your opponent. Any participant hit by a live round gets one of their up to four lives (defibrilator charges) deduced. A participant loses the game if they lose all their lives. If the shotgun empties it is reloaded with a new set of rounds.

The game gains an extra layer of strategy from the special items. At each shotgun reload both participants draw a random number of these special items from a chest. If it's the participant's turn they can use any amount of their items before they need to grab the gun to make their shot while the opponent must watch.

| Item | Special Effect |
|---|---|
| Cigarette Pack | Regain one life. |
| Magnifying glass | Know the next round in the chamber. |
| Handsaw | When applied, the shotgun has doubled damage. A sawed-off shotgun cannot be sawed off again and the state resets after each shot and finished stage. |
| Handcuffs | Skips the next turn of the opponent. Cannot be applied twice in a row. Is removed after each shotgun reload. |
| Burner phone | Know a random round after the next round in the chamber if there are any. |
| Beer | Visibly ejects the next round in the chamber. User keeps their turn. |
| Expired medicine | 50% chance of losing a life. 50% chance of gaining two. |
| Inverter | Without affecting the other rounds in the shotgun, switches the next round type in the chamber from blank to live or vice versa. The use is seen by the opponent and the round stays hidden if not already known by any participant. |
| Adrenalin | User can steal and must immediately use a chosen item of their opponent. Has a time limit when used after which the opportunity vanishes. |

The game has two game modes. The main game mode is very scripted and utilizes only a selection of items. It finishes after three stages and you can get revived which makes this mode easy to win. Therefore, this project focuses on the harder **Double or Nothing** mode. The player must win 3 stages after which they get a payout based on their finishing time. They can then decide to double the money or cash out (which is the final score). When the player decides to double the money they must win another three stages in succession, otherwise the entire money is lost and they must restart from the beginning. This is an endless mode with the goal of reaching the highest score before payout.

## Solution

### State Machine
The `State` class represents all information about the current state of the game aswell as the the next `Event`. Each evaluation phase of either partcipant is followed by either a shotgun shot or item use. Considering the fact that there are a maximum of eight rounds in the shotgun and eight items for each participant this results in a maximum of $(8 + 8 + 8) \cdot 2 = 48$ state transitions from start to end of a stage.

The transitions between states are computed in the `StateMachine` class. It returns the children of each state with their respective probability if the next event is a random event.

To immensely reduce the amount of states to be evaluated and also increase the maximum depth that can be reached in a given time frame multiple simplifying assumptions are made. For the player these only include very obvious choices like not using handcuffs in the last round or saw if the opponent has only one life left. This allows the algorithm to find non-obvious solutions like using cigarettes at full health just so that the dealer can't use them.

The dealer choices are constrained based on the actual implementation of the dealer. The dealer for example never uses beer when they know that the next round is live.
All assumptions can be looked up in `state_machine.cpp`. By setting `DEALER_USES_PLAYER_LOGIC` in `parameters.hpp` and recompiling the dealer uses the less constrained logic of the player evaluation in case one desires a harder opponent (or the multiplayer mode).

### Search
#### Expectiminimax algorithm
The search algorithm used is a variation of the Minimax algorithm commonly used in chess engines like [Stockfish](https://github.com/official-stockfish/Stockfish). It itself is a recursive depth-first-search algorithm operating on a tree. The idea is to evaluate game states based on a score and to find the moves that reach the highest end score.

Each intermediate node is assigned a score based on following rules:

* If the node is a leaf node or the maximum depth is reached the score is computed using a heuristic evaluation function.
* If it's the player's turn the score of a node is the maximum score of its child nodes. The player will always chose the option which has the most benefit for them.
* If it's the dealer's turn they will do the opposite and chose the option which has the least benefit for the player. Therefore, the node with the minimum of the child node scores is chosen.
* If a random event happens at the node that neither participant can influence then the expected value of the score is assigned. It is computed by taking the sum of all child node scores multiplied each by their probability.

The player can then backtrace on which moves to make. The problem is that the search tree grows exponentially with the available options to take and therefore optimizations are necessary to get a quick best guess. One of which has already slipped into the explanation above: The maximum depth. When it is reached the score is just "guessed" based on a evaluation function explained below and no child nodes are further evaluated.

#### Evaluation function
A choice often made in Minimax is to assign positive infinity to the win condition and negative infinity to the lose condition. This is not feasible for the Expectiminimax algorithm because it heavily skews the expected value. The score should rather represent the win probability and be therefore bounded.
The score for winning or losing a stage is therefore set to the minimum or maximum possible "advantage" value that the player can have. To increase the risk-aversity of the algorithm the loss score is a multiple of that.

The advantage a participant has is computed based on their lives. Therefore the value 1.0 resembles the player having one more life than their opponent. Equally, -1.0 means the dealer has one more life than the player.

The crux is to assign utility values to the special items like they were chess pieces (where '1' is a pawn, '3' is a knight etc.). An easy example of this are the cigarettes and the expired medicine. The expired medicine has an expected value of $0.5 \cdot (-1) + 0.5 \cdot 2 = 0.5$ lives gained per use which is half of the guaranteed life gained by using cigarettes. An obvious assumption to make is to assign double the utility to the cigarettes compared to the expired medicine.  However, since there can be 8 items per participant and only 4 lives the item advantage can dominate the life advantage and the item advantage does not accurately represent the winning probability. Therefore, the item utilities are scaled down. The utility values can be seen and modified in the `Evaluator` class.

#### Shallow and deep depth search
The search depth is split into a shallow and deep depth. For the first couple of layers not only the score is computed but also the best move and its successor moves are stored and returned. To reduce on overhead they are discarded below a certain depth (called the shallow depth). After which the algorithm continues by only computing the score until the deep depth is reached.

#### Alpha-beta pruning
This is a common and easy technique to implement which greatly reduces the amount of nodes to traverse. The idea is to stop the algorithm on a node if its upper score bound is lower than an already traversed node. The goal is to discard all options which have an obviously worse result than other nodes.

#### Transposition table
Transpositions are combinations of moves which can exist between a common start and end state. In the case of Buckshot Roulette multiple items can be used in arbitrary order with the same result. Obviously, it makes sense to store the result of traversed states in a table and read the result from that table once a transposition is encountered rather than to search the entire (possibly gigantic) sub-tree again.

This is done by utilizing a hash table. Thereforefore, a hash function for the states had to be implemented which is used to assign the states into buckets. The search with transposition table is implemented in the `TranspositionSearch` class.

#### Iterative deepening
The transposition table is especially helpful in combination with iterative deepening. The main motivation behind using iterative deepening is the problem that the computation time can be hardly estimated ahead of time. Therefore, the depth to use for the search is hard to choose in advance and cannot be modified while the search is running. Iterative deepening takes the approach of iteratively increasing the depth and restarting the search. Therefore, there will be always a result ready regardless of the timeout and using the transposition table not much time is lost in the recomputation phase. This is implemented in the `IterativeSearch` class.

#### Multithreading
The last performance optimization technique used is multithreading. However, in this project it is implemented very naively and not as advanced as it could be. Current implementation in `ThreadedSearch` will just use `std::future` to compute the algorithm for the children of the first layer. This will run the algorithm for each child in a different thread and it allows to take advantage of multi-core CPUs. However, since the computation time of those threads can also vary wildly some threads will finish sooner than others and the remaining workload is not shared to new threads. Also each thread must have their own transposition table since using a common table will result in constant blocking of the mutex. The rabbit hole of thread pools and correct Multithreading implementation is deep and might be dived into in future versions.

#### Combination of all techniques
All search classes are a template of the state machine and evaluator class used. Meaning these can be exchanged for different versions and even completely different use cases. Furthermore, the search classes are templates of each other (to limited extend) which allows the developer to build different versions of the search with or without certain features. This is done in the the `PerformanceTest` executable where they are compared by execution time.

### Game simulation
The `Game` class handles the game simulation. While the `StateMachine` returns the complete set of possible successor states extra logic must be implemented to chose one of the children as the actual successor. The class is implemented modularly, allowing the decisions made to be random, user selected or generated by the search algorithm. This allows the main executable to act both as a text-adventure version of the original game aswell as a guide on the best moves in a given scenario.

The way this is done is by implementing interfaces in the main class which are set by the constructor. These are explained below.

#### Agents
The `Agent` class implements the interface for the player and dealer strategy. There exists an `InteractiveAgent` which implements user input and an `IntelligentAgent` which implements the search algorithm mentioned above. The combination of these classes is the `InteractiveIntelligentAgent` which gives recommendations but lets the user still make the final decision while the `AutomaticIntelligentAgent` immediately executes the optimal move. Last but not least, the `RandomizedAgent` will choose a random of the available options.

#### Random events and starting configuration
For the outcomes of random events and the start state there are two possible implementations that the user can choose from. One implementation lets the user choose the events, items, lives and shotgun shells aswell one with randomized outcomes. The randomization conditions are inspired by the original game to generate authentic scenarios with accurate probabilities.

Furthermore, to be able to reproduce scenarios for debugging or benchmarking it is possible to set a seed for the random number generation. This allows the game class to run deterministically, given deterministic inputs.

## Outlook
No software project is ever truly finished. The following chapter discusses improvements that might be adressed in future versions. The end goal is to solve the 48 layers of depth for every possible starting configurations in less than a minute. Given the performance of chess engines and the complexity of chess compared to this game this should be feasible.

### Move ordering
A low hanging fruit is to order the children of each node heuristically. Alpha-beta pruning will then be more likely to prune branches and this can result in a huge performance gain. As a starting point the results of a previous run of the iterative deepening can be used which will also benedit from move ordering.

### Transposition table replacement strategy
The current transposition table is not bounded in size and for long searches this is noticable. It must be bounded and a replacement strategy implemented.

### Multithreading
As mentioned above there is a lot of room for improvement in this departement. Goal is for the software to be able to use 100% of the available system resources from start to end of the search and not vary so much in CPU usage like it does now. This can be achieved by using smarter thread handling (using thread pools) and transposition table use. There are also advanced parallel processing techniques like SIMD instructions and so on.

### User interface
The software could benefit from a better user inteface. Be it a GUI or a web interface for better portability.

### Parameter optimization
Idea is to have an automatized process for running and evaluating games to benchmark different algorithm configurations, for example to find optimal utility values for each special item. This can be done by running a genetic algorithm on a batch of games and benchmarking their performance.
