cmake_minimum_required(VERSION 3.5.0)
project(Checkers VERSION 0.1.0 LANGUAGES C CXX)

include(CTest)
include(FindPkgConfig)

find_package(nlohmann_json)

add_executable(Checkers main.cpp)

INCLUDE(FindPkgConfig)

PKG_SEARCH_MODULE(SDL2 REQUIRED sdl2)
PKG_SEARCH_MODULE(SDL2IMAGE REQUIRED SDL2_image>=2.0.0)

INCLUDE_DIRECTORIES(${SDL2_INCLUDE_DIRS} ${SDL2IMAGE_INCLUDE_DIRS})

target_link_libraries(Checkers PRIVATE nlohmann_json::nlohmann_json)
target_link_libraries(Checkers PRIVATE ${SDL2_LIBRARIES} ${SDL2IMAGE_LIBRARIES})
