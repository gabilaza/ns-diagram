cmake_minimum_required(VERSION 3.24)

project(NS-Diagram)

file(COPY src/assets DESTINATION ${CMAKE_BINARY_DIR})

find_package(SFML 2.5.1
    COMPONENTS
        system window graphics network audio REQUIRED
)

add_executable(ns_diagram src/main.cpp)
target_link_libraries(ns_diagram sfml-graphics)
target_include_directories(ns_diagram PRIVATE "src/")

