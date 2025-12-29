#include "agent.h"
#include "Driving.h"
#include "c1lidarrpi.h"
#include <vector>
#include <signal.h>
#include <iostream>
#include <stdio.h>
#include <ncurses.h>

using namespace std;

const float minDetectRange = 0.15; 
const float maxDetectRange = 1.0; 

bool running = true;
bool onestep = false;

// every 200 ms
// 8192 data points 
// (invalid: corrupted/outside detection zone)
// scan event -> actor event handler
class C1LidarScanEvent : public C1Lidar::DataInterface {
public:
    Agent& agent;

    C1LidarScanEvent(Agent& _agent) : agent(_agent) {};

public:
    void newScanAvail(C1LidarData (&data)[C1Lidar::nDistance]) {
        int nData = 0;

        vector<Observation> obs;
        for(C1LidarData &data: data) {
            Observation ob; // invalid by default
            if (data.valid && data.r > minDetectRange 
                && data.r < maxDetectRange) {
                ob.setObservation(data.x, data.y);
                nData++;
            } 
            obs.push_back(ob);
        }
        agent.eventNewRelativeCoordinates(C1Lidar::RPM/60.0f, obs);
    }
};

// closes the loop
// callback to robot actuators from current task
// initiated by actor event handler
class MotorActionEvent : public ActionInterface {
public:
    MotorActionEvent(Driving& _driving) : driving(_driving) {}

    Driving& driving;
    virtual void executeMotorAction(float l, float r) {
        driving.setMotorSpeeds(l,r);
    }
};


// terminate sig handler
void sig_handler(int signo) {
    if (signo == SIGINT) {
            running = false;
    }
}

int main(int argc, char* argv[]) { 
    signal(SIGINT, sig_handler);
    logger.startLogging("/data/deltabot/autoctrl.txt", true);

    if (argc > 1) {
        onestep = true;
    }

    Agent agent;

    C1LidarScanEvent data(agent);
    C1Lidar lidar;

    lidar.registerInterface(&data);

    try {
        logger.printf("Starting LiDAR...");
        lidar.start();
        logger.printf("SUCCESS \n");
    } 
    catch (std::exception& m) {
        logger.printf("Error: %s \n", m.what());
        lidar.stop();
        return 0;
    }

    Driving driving;

    try {
        logger.printf("Starting Motor control...");
        driving.start();
        logger.printf("SUCCESS \n");
    } 
    catch (string m) {
        logger.printf("Error: %s \n", m);
        driving.stop();
        return 0;
    }
    MotorActionEvent takeAction(driving);

    shared_ptr<AbstractTask> targetTask = make_shared<StraightTask>();

    // set up planner and initilize model  (0 straight, 1 right, -1 left)
    shared_ptr<StateMachineLTL> planner = make_shared<StateMachineLTL>(15);
    planner->setTransition(0, 1, -1);
    planner->setTransition(0, 2, 1);
    planner->setTransition(1, 3, 0);
    planner->setTransition(2, 4, 0);
    planner->setTransition(3, 5, 1);
    planner->setTransition(5, 7, 0);
    planner->setTransition(3, 9, -1);
    planner->setTransition(9, 11, 0);
    planner->setTransition(4, 6, -1);
    planner->setTransition(6, 8, 0);
    planner->setTransition(4, 10, 1);
    planner->setTransition(10, 12, 0);
    planner->setTransition(2, 13, 1);
    planner->setTransition(1, 13, -1);
    planner->setTransition(13, 14, 0);

    targetTask->registerInterface(&takeAction);
    targetTask->setInitialSpeed(0.8f);

    agent.setTargetTask(targetTask);
    agent.setPlanner(planner);

     try {
        logger.printf("Starting motors...");
        driving.start();
        logger.printf("SUCCESS \n");
    }
    catch (string m) {
        logger.printf("Error: %s \n", m);
        lidar.stop();
        driving.stop();
        return 0;
    }


    logger.startResourceLogging("/data/deltabot/usage.txt");
    
    while(running) {
        // blockingff
        int ch = getchar();
        switch (ch) {
            case 27:
                running = false;
                break;
            default:
                 break;
        }
    }

    lidar.stop();
    driving.stop();
    return 0;
}
