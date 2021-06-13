cmake_minimum_required(VERSION 3.0.0)
project(kies VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 17)

include(CTest)

add_subdirectory(third/FTXUI)

add_executable(kies main.cpp)
target_link_libraries(kies PUBLIC ftxui::component)