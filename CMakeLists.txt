cmake_minimum_required(VERSION 3.20)
project(testgame)

set(CMAKE_CXX_STANDARD 20)

find_package(SFML 3.0 COMPONENTS Graphics Window System REQUIRED)

add_executable(testgame main.cpp)

target_link_libraries(testgame SFML::Graphics SFML::Window SFML::System)