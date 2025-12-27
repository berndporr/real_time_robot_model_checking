#include "Driving.h"

void Driving::start()
{
    leftMotor.start(leftChannel, leftChipNo);
    rightMotor.start(rightChannel, rightChipNo);
}

void Driving::stop()
{
    leftMotor.setSpeed(0);
    rightMotor.setSpeed(0);
    leftMotor.stop();
    rightMotor.stop();
}

void Driving::setMotorSpeeds(float left_speed, float right_speed)
{
    leftMotor.setSpeed(left_speed);
    rightMotor.setSpeed(-right_speed);
}
