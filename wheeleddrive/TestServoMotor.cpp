#include "Driving.h"
#include "stdio.h"
// This program sets the speed of two servo motors based on command line arguments.

int main(int argc, char *argv[])
{
    if (argc < 3)
    {
        std::cout << "Usage: " << argv[0] << " <left_speed> <right_speed>(speed range from -1 to 1)" << std::endl;
        return 1;
    }

    Driving driving;
    try {
        fprintf(stderr,"Starting Motor control... ");
        driving.start();
	fprintf(stderr,"SUCCESS!\n");
    } 
    catch (string m) {
        fprintf(stderr,"Error: %s \n", m.c_str());
        driving.stop();
        return 0;
    }

    float left_speed = std::atof(argv[1]);
    float right_speed = std::atof(argv[2]);

    std::cout << "Setting speeds - Left: " << left_speed << ", Right: " << right_speed << std::endl;

    int r = driving.setMotorSpeeds(left_speed, right_speed);
    if (r < 0) {
	fprintf(stderr,"Could not set motor speed. Aborting.\n");
	driving.stop();
	return r;
    }

    std::cout << "Press any key to stop" << std::endl;

    getchar();

    driving.stop();

    return 0;
}
