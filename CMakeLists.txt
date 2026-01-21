cmake_minimum_required(VERSION 3.20)
project(testgame)

set(CMAKE_CXX_STANDARD 20)

find_package(SFML 3.0 COMPONENTS Graphics Window System REQUIRED)

include_directories(include)

add_executable(testgame MACOSX_BUNDLE
        main.cpp
        src/Game.cpp
        include/Game.h
        include/Global.h
        include/Player.h
        include/Entities.h
)

target_link_libraries(testgame SFML::Graphics SFML::Window SFML::System)

add_custom_command(TARGET testgame POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
        "${CMAKE_CURRENT_SOURCE_DIR}/assets/fonts/arial.ttf"
        "$<TARGET_FILE_DIR:testgame>/arial.ttf"
        COMMENT "Copying font to the app bundle..."
)