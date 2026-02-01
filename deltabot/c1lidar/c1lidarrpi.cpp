#include "c1lidarrpi.h"
#include <math.h>
#include <fstream>
#include <iostream>
#include <thread>
#include <chrono>

void C1Lidar::stop() {
    if (!running) return;
    running = false;
    if (th.joinable()) th.join();
    if (drv != nullptr) {
	// Stops scanning but not the motor
        drv->stop();
	// Wait for 0.2 seconds
	std::this_thread::sleep_for(std::chrono::milliseconds(200));
        // Stop the motor
        drv->stopMotor();
        RPlidarDriver::DisposeDriver(drv);
        drv = nullptr;
    }
}

void C1Lidar::start(const char* serial_port, const unsigned rpm) {
    drv = RPlidarDriver::CreateDriver(DRIVER_TYPE_SERIALPORT);
    if (!drv) throw std::runtime_error("Insufficient memory for driver");

    if (IS_OK(drv->connect(serial_port, baudrate))) {
        rplidar_response_device_info_t devinfo;
        u_result op_result = drv->getDeviceInfo(devinfo);
        if (IS_OK(op_result)) {
            rplidar_response_device_health_t healthinfo;
            op_result = drv->getHealth(healthinfo);
            if (IS_OK(op_result)) {
                if (healthinfo.status == RPLIDAR_STATUS_ERROR) {
                    std::cerr << "LIDAR internal error detected." << std::endl;
                } else {
                    // Start the scan after health is confirmed
                    drv->startScan(0, true, 0, &scanMode);
                    running = true;
                    th = std::thread(&C1Lidar::run,this);
                }
            } else {
                throw std::runtime_error("Error getting device health.");
            }
        } else {
            delete drv;
            drv = nullptr;
            throw std::runtime_error("Cannot retrieve device info");
        }
    } else {
        throw std::runtime_error("Cannot connect to the device");
    }
}



void C1Lidar::getData() {
	size_t count = (size_t)nDistance;
	rplidar_response_measurement_node_hq_t nodes[count];
	if (!drv) return;
	u_result op_result = drv->grabScanDataHq(nodes, count);
	if (SL_IS_OK(op_result)) {
		drv->ascendScanData(nodes, count);
		for (int pos = 0; pos < (int)count ; ++pos) {
			float angle = - nodes[pos].angle_z_q14 * (90.f / 16384.f / (180.0f / M_PI));
			float dist = nodes[pos].dist_mm_q2/4000.0f;
			if (dist > 0) {
				//fprintf(stderr,"%d,phi=%f,r=%f\n",j,angle,dist);
				c1LidarData[currentBufIdx][pos].phi = angle;
				c1LidarData[currentBufIdx][pos].r = dist;
				c1LidarData[currentBufIdx][pos].x = cos(angle) * dist;
				c1LidarData[currentBufIdx][pos].y = sin(angle) * dist;
				c1LidarData[currentBufIdx][pos].signal_strength =
					nodes[pos].quality >> RPLIDAR_RESP_MEASUREMENT_QUALITY_SHIFT;
				c1LidarData[currentBufIdx][pos].valid = true;
				dataAvailable = true;
			} else {
				c1LidarData[currentBufIdx][pos].valid = false;
			}
		}
		if ( (dataAvailable) && (nullptr != dataInterface) ) {
			dataInterface->newScanAvail(c1LidarData[currentBufIdx]);
		}
		readoutMtx.lock();
		currentBufIdx = !currentBufIdx;
		readoutMtx.unlock();
	}
}

void C1Lidar::run() {
    while (running) {
        getData();  // Keep collecting data while the system is running
    }
}
