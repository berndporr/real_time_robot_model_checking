#ifndef DRIVING_H
#define DRIVING_H

#include "ServoMotorSetting.h"

class Driving
{
public:
    /**
     * @brief Inits the motors and switches on pwm
     * Throws an exeption if not successful.
     */
    void start();

    /**
     * @brief Sets the speeds of the motors
     * This sets the speeds in a way that a positive speed = forward
     * and a neg speed is backwards.
     * 
     * @param left_speed A value between -1 and +1
     * @param right_speed A speed value between -1 and +1
     */
    int setMotorSpeeds(float left_speed, float right_speed);

    /**
     * Stop the robot and switch off PWM.
     **/
    void stop();

private:
    // Servo motor settings for left and right motors
    ServoMotorSetting leftMotor;
    ServoMotorSetting rightMotor;

    // Constants for servo motor settings
    static constexpr int leftChannel = 0;
    static constexpr int rightChannel = 0;
    static constexpr int leftChipNo = 2;
    static constexpr int rightChipNo = 1;
};

#endif
