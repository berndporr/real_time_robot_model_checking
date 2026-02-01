#include "c1lidarrpi.h"
#include <iostream>
#include <fstream>  // Include for file handling
#include <thread>   // For sleep functionality
#include <chrono>   // For timing


class DataInterface : public C1Lidar::DataInterface {
public:
	void newScanAvail(C1LidarData (&data)[C1Lidar::nDistance]) {
	    auto now = std::chrono::system_clock::now();
	    auto duration = now.time_since_epoch();
	    auto milliseconds
		= std::chrono::duration_cast<std::chrono::milliseconds>(
									duration)
		.count();
	    std::cerr << milliseconds << std::endl;
		for (C1LidarData &d : data) {
			if (d.valid) {  // Only process valid data
			    std::cout << d.x << "\t" << d.y 
				      << "\t" << d.r << "\t" << d.phi 
				      << "\t" << d.signal_strength << std::endl;
			}
		}
	}
};

int main(int, char **) {
    fprintf(stderr, "Data format: x <tab> y <tab> r <tab> phi <tab> strength\n");
    fprintf(stderr, "Press any key to stop.\n");

    C1Lidar lidar;
    DataInterface dataInterface;
    lidar.registerInterface(&dataInterface);  // Registers the callback function

    try {
        lidar.start("/dev/ttyS2");  // Start the LIDAR
    } catch (const char* msg) {
        std::cerr << "ERROR: " << msg << std::endl;
        lidar.stop(); // Make sure motor and scan stop
        return 1;
    }

    // waiting for a keypress
    getchar();
    
    std::cerr << "\nStopped cleanly." << std::endl;
    lidar.stop();  // Stop the scanning and motor
    return 0;
}
