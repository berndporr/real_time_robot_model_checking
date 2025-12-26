#include "agent.h"
#include "alphabot.h"
#include "a1lidarrpi.h"
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
class A1LidarScanEvent : public A1Lidar::DataInterface {
public:
    Agent& agent;

    A1LidarScanEvent(Agent& _agent) : agent(_agent) {};

public:
    void newScanAvail(float rpm, A1LidarData (&data)[A1Lidar::nDistance]) {
        int nData = 0;

        vector<Observation> obs;
        for(A1LidarData &data: data) {
            Observation ob; // invalid by default
            if (data.valid && data.r > minDetectRange 
                && data.r < maxDetectRange) {
                ob.setObservation(data.x, data.y);
                nData++;
            } 
            obs.push_back(ob);
        }
        agent.eventNewRelativeCoordinates(rpm/60.0f, obs);
    }
};

// closes the loop
// callback to robot actuators from current task
// initiated by actor event handler
class MotorActionEvent : public ActionInterface {
public:
    MotorActionEvent(AlphaBot& _alphabot) : alphabot(_alphabot) {}

    AlphaBot& alphabot;
    virtual void executeMotorAction(float l, float r) {
        alphabot.setLeftWheelSpeed(l);
        alphabot.setRightWheelSpeed(r);
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
    logger.startLogging("../10/autoctrl.txt", true);

    if (argc > 1) {
        onestep = true;
    }

    Agent agent;

    A1LidarScanEvent data(agent);
    A1Lidar lidar;

    lidar.registerInterface(&data);

    try {
        logger.printf("Starting LiDAR...");
        lidar.start();
        logger.printf("SUCCESS \n");
    } 
    catch (string m) {
        logger.printf("Error: %s \n", m);
        lidar.stop();
        return 0;
    }

    AlphaBot alphabot;
    MotorActionEvent takeAction(alphabot);

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
        alphabot.start();
        logger.printf("SUCCESS \n");
    }
    catch (string m) {
        logger.printf("Error: %s \n", m);
        lidar.stop();
        alphabot.stop();
        return 0;
    }

    logger.startResourceLogging("../10/usage.txt");
    
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
    alphabot.stop();
    return 0;
}
