cmake_minimum_required(VERSION 3.1.0)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
	set(CMAKE_BUILD_TYPE "RelWithDebInfo")
endif()

project(Alphabot_Library LANGUAGES CXX)
include(GNUInstallDirs)
set(CMAKE_CXX_STANDARD 11)

add_compile_options(-Wall)

find_package (Curses)

set(LIBSRC alphabot.cpp)
set(LIBINCLUDE alphabot.h)

add_library(alphabot
  STATIC
  ${LIBSRC}
  )

set_target_properties(alphabot PROPERTIES
  POSITION_INDEPENDENT_CODE TRUE
  PUBLIC_HEADER "${LIBINCLUDE}")

target_link_libraries(alphabot pigpio rt)

install(TARGETS alphabot EXPORT alphabot-targets
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
  ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
  PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})

add_executable (testIO testIO.cpp)
target_link_libraries(testIO alphabot ncurses rt)

add_executable (testMotor testMotor.cpp)
target_link_libraries(testMotor alphabot ncurses rt)

add_executable (obstacleAvoid obstacleAvoid.cpp)
target_link_libraries(obstacleAvoid alphabot ncurses rt)
