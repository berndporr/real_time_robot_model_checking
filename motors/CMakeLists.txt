cmake_minimum_required(VERSION 3.20)
project(deltabot)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(CMAKE_BUILD_TYPE Debug)

add_library(deltabotmotors
  STATIC
  Pwm.cpp
  Driving.cpp
  ServoMotorSetting.cpp
  )

add_executable(TestServoMotor 
    TestServoMotor.cpp
)

target_link_libraries(TestServoMotor deltabotmotors)
