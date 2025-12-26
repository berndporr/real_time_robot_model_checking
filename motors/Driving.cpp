#include "Driving.h"

Driving::Driving()
    : leftMotor(leftChannel, low_time, high_time, leftChipNo),
      rightMotor(rightChannel, low_time, high_time, rightChipNo)
{
}

void Driving::Forward(float speed) // Move the robot forward
{
    SetMotorSpeed(speed, speed);
}

void Driving::Backward(float speed) // Move the robot backward
{
    SetMotorSpeed(-speed, -speed);
}

void Driving::TurnLeft(float speed) // Turn the robot to the left
{
    SetMotorSpeed(speed / 2.0f, speed);
}

void Driving::TurnRight(float speed) // Turn the robot to the right
{
    SetMotorSpeed(speed, speed / 2.0f);
}

void Driving::Stop() // Stop the robot
{
    leftMotor.StopMotor();
    rightMotor.StopMotor();
}

void Driving::SetMotorSpeed(float left_speed, float right_speed)
{
    leftMotor.ChangeLeftSpeed(left_speed);
    rightMotor.ChangeRightSpeed(-right_speed);
}
