cmake_minimum_required(VERSION 3.10)
project(Comp345_Assignment1 CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Every game class. The classes depend on each other (an order uses players and
# territories, the engine uses everything, all loggable classes use the observer),
# so each driver is built with the whole set.
set(GAME_SOURCES
        part2/Map.cpp
        part2/Map.h
        part2/Player.cpp
        part2/Player.h
        part2/Cards.cpp
        part2/Cards.h
        part2/Orders.cpp
        part2/Orders.h
        part2/GameEngine.cpp
        part2/GameEngine.h
        part2/CommandProcessing.cpp
        part2/CommandProcessing.h
        part2/LoggingObserver.cpp
        part2/LoggingObserver.h
)

add_executable(MapDriver
        ${GAME_SOURCES}
        part2/MapDriver.cpp
)

# Lets MapDriver find part2/maps/ no matter which directory it is run from.
target_compile_definitions(MapDriver PRIVATE MAPS_DIR="${CMAKE_CURRENT_SOURCE_DIR}/part2/maps/")


add_executable(CardsDriver
        ${GAME_SOURCES}
        part2/CardsDriver.cpp
)

add_executable(OrdersDriver
        ${GAME_SOURCES}
        part2/OrdersDriver.cpp
)

add_executable(PlayerDriver
        ${GAME_SOURCES}
        part2/PlayerDriver.cpp
)

add_executable(GameEngineDriver
        ${GAME_SOURCES}
        part2/GameEngineDriver.cpp
)

add_executable(MainDriver
        main.cpp)

# Assignment 2, Part 1. Run with no option (uses part2/commands.txt),
# with -console, or with -file <filename>
add_executable(CommandProcessingDriver
        ${GAME_SOURCES}
        part2/CommandProcessingDriver.cpp
)
target_compile_definitions(CommandProcessingDriver PRIVATE COMMANDS_FILE="${CMAKE_CURRENT_SOURCE_DIR}/part2/Commands.txt")

# Assignment 2, Part 5. Run with no option (uses part2/LoggingCommands.txt),
# with -console, or with -file <filename>. Writes part2/gamelog.txt.
add_executable(LoggingObserverDriver
        ${GAME_SOURCES}
        part2/LoggingObserverDriver.cpp
)
target_compile_definitions(LoggingObserverDriver PRIVATE PART2_DIR="${CMAKE_CURRENT_SOURCE_DIR}/part2")
