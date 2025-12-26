cmake_minimum_required(VERSION 3.10)

project(ObstacleAvoid)

find_package(Threads)
find_package(CURL REQUIRED) 

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_compile_options(-Wall)

include(GNUInstallDirs)

include_directories(${CURL_INCLUDE_DIR})

add_subdirectory(agent)
add_subdirectory(motors)
add_subdirectory(rplidar_rock)

add_executable(autoctrl autoctrl.cpp)
target_link_libraries(autoctrl c1lidarrpi deltabotmotors pthread agent ncurses)
