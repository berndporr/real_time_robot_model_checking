cmake_minimum_required(VERSION 3.20.0)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE "RelWithDebInfo")
endif()

project(RPLIDAR_C1_Rock5B LANGUAGES CXX)
include(GNUInstallDirs)
set(CMAKE_CXX_STANDARD 17)

find_package(Threads REQUIRED)

include_directories(rplidarsdk/sdk/include rplidarsdk/sdk/src/)

set(LIBSRC
  c1lidarrpi.cpp
  rplidarsdk/sdk/src/rplidar_driver.cpp
  rplidarsdk/sdk/src/sl_lidar_driver.cpp
  rplidarsdk/sdk/src/arch/linux/net_socket.cpp
  rplidarsdk/sdk/src/arch/linux/timer.cpp
  rplidarsdk/sdk/src/arch/linux/net_serial.cpp
  rplidarsdk/sdk/src/sl_udp_channel.cpp
  rplidarsdk/sdk/src/hal/thread.cpp
  rplidarsdk/sdk/src/sl_async_transceiver.cpp
  rplidarsdk/sdk/src/sl_lidarprotocol_codec.cpp
  rplidarsdk/sdk/src/sl_serial_channel.cpp
  rplidarsdk/sdk/src/dataunpacker/unpacker/handler_capsules.cpp
  rplidarsdk/sdk/src/dataunpacker/unpacker/handler_hqnode.cpp
  rplidarsdk/sdk/src/dataunpacker/unpacker/handler_normalnode.cpp
  rplidarsdk/sdk/src/dataunpacker/dataunpacker.cpp
  rplidarsdk/sdk/src/sl_crc.cpp
  rplidarsdk/sdk/src/sl_tcp_channel.cpp
)

add_library(c1lidarrpi STATIC ${LIBSRC})

set_target_properties(c1lidarrpi PROPERTIES
  POSITION_INDEPENDENT_CODE TRUE
  PUBLIC_HEADER c1lidarrpi.h)

target_link_libraries(c1lidarrpi Threads::Threads rt)

add_executable(printdata printdata.cpp)
target_link_libraries(printdata c1lidarrpi Threads::Threads)

install(TARGETS c1lidarrpi EXPORT c1lidarrpi_targets
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
  ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
  PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
  INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
  )

install(DIRECTORY rplidarsdk/ DESTINATION include/rplidarsdk FILES_MATCHING PATTERN "*.h")

install(EXPORT c1lidarrpi_targets
  DESTINATION lib/cmake/C1lidarRPI
  FILE C1lidarRPIConfig.cmake
)
