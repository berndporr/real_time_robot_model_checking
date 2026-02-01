#include "alphabot.h"
#include <pigpio.h>
#include <math.h>
#include <iostream>
#include <unistd.h>


void AlphaBot::initPWM(int gpio) {
        gpioSetPWMfrequency(gpio,50);
        int rr = gpioGetPWMrealRange(gpio);
        if ( ( rr > 255) && (rr < 20000) ) gpioSetPWMrange(gpio, rr);
        setLeftWheelSpeed(0);
        setRightWheelSpeed(0);
}

void AlphaBot::start(long _samplingInterval, int _timerNumber) {
	int cfg = gpioCfgGetInternals();
	cfg |= PI_CFG_NOSIGHANDLER;
	gpioCfgSetInternals(cfg);
	int r = gpioInitialise();
	if (r < 0) {
		char msg[] = "Cannot init pigpio.";
		std::cerr << msg << "\n";
		throw msg;
	}

        initPWM(GPIO_MOTORL);
        initPWM(GPIO_MOTORR);

        // collision sensor
        gpioSetMode(GPIO_COLLISION_L,PI_INPUT);
        gpioSetMode(GPIO_COLLISION_R,PI_INPUT);

        // ADC
        gpioSetMode(GPIO_ADC_CS,PI_OUTPUT);
        gpioWrite(GPIO_ADC_CS,1);
        gpioSetMode(GPIO_ADC_ADDR,PI_OUTPUT);
        gpioSetMode(GPIO_ADC_IOCLK,PI_OUTPUT);
        gpioSetMode(GPIO_ADC_DOUT,PI_INPUT);

        samplingInterval = _samplingInterval;
	timerNumber = _timerNumber;
	r = gpioSetTimerFuncEx(timerNumber,samplingInterval,pigpioTimerCallback,(void*)this);
	if (r < 0) {
		char msg[] = "Cannot start timer.";
		std::cerr << msg << "\n";
		throw msg;
	}
}

void AlphaBot::stop() {
	gpioSetTimerFuncEx(timerNumber,samplingInterval,NULL,(void*)this);
        setLeftWheelSpeed(0);
        setRightWheelSpeed(0);
	gpioTerminate();
}

unsigned AlphaBot::readADC(unsigned channel) {
	unsigned us = 1;
        gpioWrite(GPIO_ADC_CS,0);
	usleep(us);
	channel = channel << 4;
	for (unsigned i = 0; i < 4; i ++) {
		if(channel & 0x80)
			gpioWrite(GPIO_ADC_ADDR,1);
		else 
			gpioWrite(GPIO_ADC_ADDR,0);
		usleep(us);
		gpioWrite(GPIO_ADC_IOCLK,1);
		usleep(us);
		gpioWrite(GPIO_ADC_IOCLK,0);
		usleep(us);
		channel = channel << 1;
	}
	for (unsigned i = 0; i < 6;i ++) {
		usleep(us);
		gpioWrite(GPIO_ADC_IOCLK,1);
		usleep(us);
		gpioWrite(GPIO_ADC_IOCLK,0);
	}

        gpioWrite(GPIO_ADC_CS,1);
	usleep(us);
	gpioWrite(GPIO_ADC_CS,0);
	
        unsigned value = 0; 
	for (unsigned i = 0; i < 2; i ++) {
		gpioWrite(GPIO_ADC_IOCLK,1);
		usleep(us);
		value <<= 1;
		usleep(us);
		if (gpioRead(GPIO_ADC_DOUT))
			value |= 0x1;
		gpioWrite(GPIO_ADC_IOCLK,0);
		usleep(us);
	} 

	for (unsigned i = 0; i < 8; i ++) {
		gpioWrite(GPIO_ADC_IOCLK,1);
		usleep(us);
		value <<= 1;
		if (gpioRead(GPIO_ADC_DOUT))
			value |= 0x1;
		gpioWrite(GPIO_ADC_IOCLK,0);
		usleep(us);
	}
	usleep(us);
        gpioWrite(GPIO_ADC_CS,1);

	return value; 
}
	

void AlphaBot::timerEvent() {
	// distance readings
	leftDistance = readADC(ADC_DIST_L);
	rightDistance = readADC(ADC_DIST_R);
	
	// battery
	batteryLevel = readADC(ADC_VIN);
	
	// IR sensors
	for(unsigned i=0;i<(nIR);i++) {
		ir[i] = (float)(readADC(i)) / ADCmax;
	}
	
	// callback
	if (nullptr != stepCallback)
		stepCallback->step(*this);
	
}

void AlphaBot::setRightWheelSpeed(float speed) {
	speed = -speed;
        if (speed < -1)
                speed = -1;
        if (speed > 1)
                speed = 1;
        rightWheelSpeed = speed;
        float max = (float)gpioGetPWMrange(GPIO_MOTORR);
	speed = speed * speed2pwm;
        gpioPWM(GPIO_MOTORR,(int)round((motor_rest_pulseduration+speed)*max));
}

void AlphaBot::setLeftWheelSpeed(float speed) {
        if (speed < -1)
                speed = -1;
        if (speed > 1)
                speed = 1;
        leftWheelSpeed = speed;
        float max = (float)gpioGetPWMrange(GPIO_MOTORL);
	speed = speed * speed2pwm;	
        gpioPWM(GPIO_MOTORL,(int)round((motor_rest_pulseduration+speed)*max));
}


bool AlphaBot::getCollisionLeft() {
        return gpioRead(GPIO_COLLISION_L) == 0; 
}

bool AlphaBot::getCollisionRight() {
        return gpioRead(GPIO_COLLISION_R) == 0; 
}

