cmake_minimum_required(VERSION 3.19)
project(buckshot-roulette-solver VERSION 2.0.0)

# parameters
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

if(NOT CMAKE_BUILD_TYPE)
   set(CMAKE_BUILD_TYPE Release)
endif()
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3")

# library
add_library(br-engine STATIC src/objects/magazine.cpp
                             src/objects/shotgun.cpp
                             src/objects/participant.cpp
                             src/objects/state.cpp
                             src/agents/randomized_agent.cpp
                             src/agents/intelligent_agent.cpp
                             src/agents/interactive_agent.cpp
                             src/agents/automatic_intelligent_agent.cpp
                             src/agents/interactive_intelligent_agent.cpp
                             src/string_functions.cpp
                             src/item_drawers/get_input_item_drawer.cpp
                             src/item_drawers/randomized_item_drawer.cpp
                             src/evaluator.cpp
                             src/game.cpp
                             src/interactive_game.cpp
                             src/state_machine.cpp)

# linking
target_include_directories(br-engine PUBLIC
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:include>
)
add_executable(BRengine src/main.cpp)
target_link_libraries(BRengine br-engine)
add_executable(BRsimulation src/simulation.cpp)
target_link_libraries(BRsimulation br-engine)

# TESTING -------------------------------------------------------
option(ENABLE_TESTING "Enable the tests" ON)
if(ENABLE_TESTING)
    include(CTest)
    enable_testing()

    # Fetch Catch2
    include(FetchContent)
    FetchContent_Declare(
        Catch2
        GIT_REPOSITORY https://github.com/catchorg/Catch2.git
        GIT_TAG v3.4.0  # or the latest version
        FIND_PACKAGE_ARGS
    )
    FetchContent_MakeAvailable(Catch2)
    include(Catch)
    set_target_properties(Catch2 PROPERTIES CMAKE_BUILD_TYPE Release)

    # add test executable
    add_executable(ExpectiminimaxTest test/expectiminimax_test.cpp)
    target_link_libraries(ExpectiminimaxTest br-engine Catch2::Catch2)
    set_target_properties(ExpectiminimaxTest PROPERTIES CMAKE_BUILD_TYPE Debug)

    add_executable(PerformanceTest test/performance_test.cpp)
    target_link_libraries(PerformanceTest br-engine Catch2::Catch2)
    set_target_properties(PerformanceTest PROPERTIES CMAKE_BUILD_TYPE Release)

    add_executable(HashTest test/hash_test.cpp)
    target_link_libraries(HashTest br-engine Catch2::Catch2)
    set_target_properties(HashTest PROPERTIES CMAKE_BUILD_TYPE Debug)

    add_executable(GameTest test/game_test.cpp)
    target_link_libraries(GameTest br-engine Catch2::Catch2)
    set_target_properties(GameTest PROPERTIES CMAKE_BUILD_TYPE Debug)

    # simple unit test
    add_test(NAME HashTest COMMAND HashTest)
    add_test(NAME ExpectiminimaxTest COMMAND ExpectiminimaxTest)
    add_test(NAME GameTest COMMAND GameTest)
    add_test(NAME PerformanceTest COMMAND PerformanceTest)

endif()