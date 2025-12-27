#include "Driving.h"

int Driving::start()
{
    int r = 0;
    r = leftMotor.start(leftChannel, leftChipNo);
    if (r < 0) {
	fprintf(stderr,"Could not start left motor.\n");
	stop();
	return r;
    }
    r = rightMotor.start(rightChannel, rightChipNo);
    if (r < 0) {
	fprintf(stderr,"Could not start right motor.\n");
	stop();
	return r;
    }
    setMotorSpeeds(0,0);
    return r;
}

void Driving::stop()
{
    leftMotor.setSpeed(0);
    rightMotor.setSpeed(0);
    leftMotor.stop();
    rightMotor.stop();
}

int Driving::setMotorSpeeds(float left_speed, float right_speed)
{
    int r = 0;
    r = leftMotor.setSpeed(left_speed);
    if (r < 0) {
	fprintf(stderr,"Could not set left motor speed.\n");
	return r;
    }
    r = rightMotor.setSpeed(-right_speed);
    if (r < 0) {
	fprintf(stderr,"Could not set right motor speed.\n");
	return r;
    }
    return r;
}
