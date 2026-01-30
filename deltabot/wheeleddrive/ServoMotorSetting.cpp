#include "ServoMotorSetting.h"

int ServoMotorSetting::start(int channel, int chipNo)
{
    return pwm.start(channel, low_time, high_time, chipNo);
    setSpeed(0);
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

int ServoMotorSetting::setSpeed(float speed)
{
    int r;
    int high_time = speedToHighTime(speed);
    int per = INVERSEDUTY + high_time;
    r = pwm.setPeriod(per);
    if ( r < 0 ) {
	fprintf(stderr,"Cannot set period.\n");
	return r;
    }
    r = pwm.setDutyCycleNS(INVERSEDUTY);
    if ( r < 0 ) {
	fprintf(stderr,"Cannot set duty cycle.\n");
    }
    return r;
}

int ServoMotorSetting::stop()
{
    int r;
    r = setSpeed(0);
    if (r < 0) {
	fprintf(stderr,"Could not set speed to zero.\n");
    }
    r = pwm.disable();
    if (r < 0) {
	fprintf(stderr,"Could not disable the PWM.\n");
    }
    return r;
}
