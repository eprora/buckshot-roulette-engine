#include "engine/interactive_game.hpp"
#include <iostream>

// ------------------------------MAIN-------------------------------------------------
int main(int argc, char* argv[]){				// argc = number of arguments, argv = array of argument c-strings (argv[1] is the first)
    
    engine::InteractiveGame game;
    
    while(true) {
        game.start();
        while(!game.isFinished()) {
            game.playMove();
        }
    }

	return 0;
}