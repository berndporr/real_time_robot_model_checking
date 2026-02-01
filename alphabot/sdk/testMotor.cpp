#include "alphabot.h"
#include <unistd.h>
#include <cstdio>

int running = 1;

int main(int, char **)
{
        AlphaBot alphabot;
        alphabot.start();
	float l = 0;
	float r = 0;
	float d = 0.0101f;
        for(int i = 0; i < 100; i++) 
        {
		if (i < 50) {
			l = l + d;
		} else {
			l = l - d;
		}
		alphabot.setLeftWheelSpeed(l);
		usleep(100000);
		if (i < 40) {
			r = r + d;
		} else {
			r = r - d;
		}
		alphabot.setRightWheelSpeed(r);
		usleep(100000);
		printf(" L = %f \t R= %f\n",l,r);
        }
        alphabot.stop();
}
