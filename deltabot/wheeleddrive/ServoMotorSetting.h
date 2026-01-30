#ifndef SERVO_MOTOR_SETTING_H
#define SERVO_MOTOR_SETTING_H
#include <iostream>
#include "Pwm.h"

class ServoMotorSetting
{
public:
    /**
     * ServoMotorSetting 
     * 
     * @param  {int} channel     : The GPIO pin number for the motor
     * @param  {int} chipNo      : The chip number for the motor
     */
    int start(int channel, int chipNo);

    ~ServoMotorSetting();

    /**
     * Change the speed of the motor.
     * @param  {float} speed : The speed to set for the motor.
     */
    int setSpeed(float speed);
    
    /**
     * Switch off pwm
     */
    int stop();

private:
    PWM pwm; // Class that handles PWM operations
    /**
     * Convert speed to high time for PWM signal.
     * @param  {float} speed : The speed to convert.
     * @return {int}         : The high time duration for the PWM signal.
     */
    int speedToHighTime(float speed);

    static constexpr float low_time = 20.0f;
    static constexpr float high_time = 1.52f;
    static constexpr int INVERSEDUTY = 20000000;
};

#endif
