#include "ServoMotorSetting.h"

void ServoMotorSetting::start(int channel, int chipNo)
{
    pwm.start(channel, low_time, high_time, chipNo);
}

ServoMotorSetting::~ServoMotorSetting()
{
    stop();
}

int ServoMotorSetting::speedToHighTime(float speed) // speed range from -1 to 1, 0 is stop
{
    if (speed > 1)
        speed = 1;
    if (speed < -1)
        speed = -1;
    int neutral_time = 1524000; // When speed is 0, The high time is 1524000
    int step_size = 100000;

    return neutral_time + (speed * step_size);
}

void ServoMotorSetting::setSpeed(float speed)
{
    int high_time = speedToHighTime(speed);
    int per = INVERSEDUTY + high_time;
    pwm.setPeriod(per);
    pwm.setDutyCycleNS(INVERSEDUTY);
}

void ServoMotorSetting::stop()
{
    setSpeed(0);
    pwm.disable();
}
