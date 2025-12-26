cmake_minimum_required(VERSION 3.20)
project(deltabot)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_library(deltabotmotors
  STATIC
  Pwm.cpp
  Driving.cpp
  ServoMotorSetting.cpp
  )

target_include_directories(deltabotmotors PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

add_executable(TestServoMotor 
    TestServoMotor.cpp
)

target_link_libraries(TestServoMotor deltabotmotors)
