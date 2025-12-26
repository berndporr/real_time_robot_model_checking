#include "DeltaBot.h"

DeltaBot::DeltaBot()
    : leftMotor(leftChannel, low_time, high_time, leftChipNo),
      rightMotor(rightChannel, low_time, high_time, rightChipNo)
{
}

void DeltaBot::Forward(float speed) // Move the robot forward
{
    SetMotorSpeed(speed, speed);
}

void DeltaBot::Backward(float speed) // Move the robot backward
{
    SetMotorSpeed(-speed, -speed);
}

void DeltaBot::TurnLeft(float speed) // Turn the robot to the left
{
    SetMotorSpeed(speed / 2.0f, speed);
}

void DeltaBot::TurnRight(float speed) // Turn the robot to the right
{
    SetMotorSpeed(speed, speed / 2.0f);
}

void DeltaBot::Stop() // Stop the robot
{
    leftMotor.StopMotor();
    rightMotor.StopMotor();
}

void DeltaBot::SetMotorSpeed(float left_speed, float right_speed)
{
    leftMotor.ChangeLeftSpeed(left_speed);
    rightMotor.ChangeRightSpeed(-right_speed);
}
