#include "alphabot.h"
#include <ncurses.h>
#include <iostream>

int running = 1;

// callback every 100ms
class DisplaySensorCallback : public AlphaBot::StepCallback
{
public:
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
	mvaddstr(0, 0, "l)eft wheel, r)ight wheel, f)orward, b)ackward, SPACE=stop, ESC=end");
	refresh();
	alphabot.start();
	float l = 0;
	float r = 0;
	char tmp[256];
	while (running)
	{
		// blocking so that the main program sleeps here
		int ch = getchar();
		switch (ch)
		{
		case 27:
			running = 0;
			break;

		case 'l':
			l = l + 0.1f;
			alphabot.setLeftWheelSpeed(l);
			sprintf(tmp, "Increasing right speed to %f         ", l);
			mvaddstr(6, 0, tmp);
			refresh();
			break;

		case 'r':
			r = r + 0.1f;
			alphabot.setRightWheelSpeed(r);
			sprintf(tmp, "Increasing right speed to %f         ", r);
			mvaddstr(6, 0, tmp);
			refresh();
			break;

		case 'b':
			l = l - 0.05f;
			r = r - 0.05f;
			alphabot.setLeftWheelSpeed(l);
			alphabot.setRightWheelSpeed(r);
			sprintf(tmp,"Backwards       l=%f, r=%f                     ",l,r);
			mvaddstr(6, 0, tmp);
			refresh();
			break;

		case 'f':
			l = l + 0.05f;
			r = r + 0.05f;
			alphabot.setLeftWheelSpeed(l);
			alphabot.setRightWheelSpeed(r);
			sprintf(tmp,"Forwards       l=%f, r=%f                     ",l,r);
			mvaddstr(6, 0, tmp);
			refresh();
			break;

		case ' ':
			l = 0;
			r = 0;
			alphabot.setLeftWheelSpeed(l);
			alphabot.setRightWheelSpeed(r);
			mvaddstr(6, 0, "Stopping                            ");
			refresh();
			break;

		default:
			break;
		}
	}
	alphabot.stop();
	endwin();
}
