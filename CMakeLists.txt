cmake_minimum_required(VERSION 3.0.0)
project(temperature VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 17)

include(CTest)

add_subdirectory(third/FTXUI)

add_executable(temperature main.cpp)
target_link_libraries(temperature PUBLIC ftxui::component)

install(TARGETS temperature DESTINATION ${CMAKE_INSTALL_PREFIX}/bin)