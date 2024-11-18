cmake_minimum_required(VERSION 3.28)
project(memdb)

set(CMAKE_CXX_STANDARD 17)

add_executable(memdb main.cpp)


find_package(nlohmann_json 3.2.0 REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE nlohmann_json::nlohmann_json)
