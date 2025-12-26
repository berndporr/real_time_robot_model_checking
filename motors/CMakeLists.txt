cmake_minimum_required(VERSION 3.20)
project(deltabot)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(CMAKE_BUILD_TYPE Debug)

find_package(Threads REQUIRED)
find_package(OpenCL REQUIRED)

add_subdirectory(rplidar_rock)

set(COMMON_SOURCES
    Pwm.cpp
    ServoMotorSetting.cpp
    DeltaBot.cpp
)

add_executable(TestServoMotor 
    TestServoMotor.cpp
    ${COMMON_SOURCES}
    ${CLDL_SOURCES}
)

add_executable(TestSpeedChange 
    TestSpeedChange.cpp
    ${COMMON_SOURCES}
    ${CLDL_SOURCES}
)
