#include "alphabot.h"
#include <ncurses.h>
#include <iostream>

int running = 1;

// callback every 100ms
class DisplaySensorCallback : public AlphaBot::StepCallback
{
public:
	float l = 0.3;
	float r = 0.3;
	virtual void step(AlphaBot &alphabot)
	{
		char tmp[256];
		sprintf(tmp, "Coll L = %d, Coll R = %d       ", alphabot.getCollisionLeft(), alphabot.getCollisionRight());
		mvaddstr(1, 0, tmp);
		sprintf(tmp,
				"Dist L = %1.4f, Dist R = %1.4f, Power = %1.1f Volt    ",
				alphabot.getLeftDistance(),
				alphabot.getRightDistance(),
				alphabot.getBatteryLevel());
		mvaddstr(3, 0, tmp);
		for(unsigned i = 0; i < alphabot.nIR; i++) {
			sprintf(tmp,"IR%d = %1.4f  ",i,alphabot.getIR()[i]);
			mvaddstr(4,i*15,tmp);
		}
		float distL = alphabot.getLeftDistance() / 0.28;
		float distR = alphabot.getRightDistance() / 0.25;
		float phi = distL - distR;
		sprintf(tmp,"phi = %f",phi);
		mvaddstr(6,0,tmp);
		bool collision = alphabot.getCollisionLeft() || alphabot.getCollisionRight();
		if (collision)
		{
			alphabot.setLeftWheelSpeed(0);
			alphabot.setRightWheelSpeed(0);
		}
		else
		{
			alphabot.setLeftWheelSpeed(l + 0.2 * phi);
			alphabot.setRightWheelSpeed(r - 0.2 * phi);
		}
		refresh();
	}
};

int main(int, char **)
{
	AlphaBot alphabot;
	DisplaySensorCallback displaySensorCallback;
	alphabot.registerStepCallback(&displaySensorCallback);
	initscr();
	noecho();
	clear();
	mvaddstr(0, 0, "ESC=end");
	refresh();
	alphabot.start();
	while (running)
	{
		// blocking so that the main program sleeps here
		int ch = getchar();
		switch (ch)
		{
		case 27:
			running = 0;
			break;

		default:
			break;
		}
	}
	alphabot.stop();
	endwin();
}
