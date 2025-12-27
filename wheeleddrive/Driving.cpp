#include "Driving.h"

void Driving::start()
{
    int r = 0;
    r = leftMotor.start(leftChannel, leftChipNo);
    if (r < 0) {
	stop();
	throw "Driving: could not start the left motor!";
    }
    r = rightMotor.start(rightChannel, rightChipNo);
    if (r < 0) {
	stop();
	throw "Driving: could not start the right motor!";
    }
    setMotorSpeeds(0,0);
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
