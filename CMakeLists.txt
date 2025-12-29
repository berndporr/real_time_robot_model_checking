cmake_minimum_required(VERSION 3.10)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE "RelWithDebInfo")
endif()

project(ObstacleAvoid)

find_package(Threads)
find_package(CURL REQUIRED) 

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_compile_options(-Wall)

include(GNUInstallDirs)

include_directories(${CURL_INCLUDE_DIR})

add_subdirectory(agent)
add_subdirectory(wheeleddrive)
add_subdirectory(rplidar_rock)

add_executable(autoctrl autoctrl.cpp)
target_link_libraries(autoctrl c1lidarrpi deltabotdrive pthread agent ncurses)
