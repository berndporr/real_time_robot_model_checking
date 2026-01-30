#ifndef AGENT_H
#define AGENT_H

// todo: review
#include "logger.h"
#include <vector>
#include <cassert>
#include <math.h>
#include <memory>
#include <string>
#include <stack>
#include <set>
#include <algorithm>
#include <random>
#include <list>
#include <cstdio>
#include <cmath>
#include <string>
#include <chrono>

using namespace std;
using namespace std::chrono;

extern int nEvents;
extern float detectionThreshold;
extern bool onestep;

const char run[] = "10";
const char west[] = "west";
const char east[] = "east";
const char northWest[] = "northWest";
const char southWest[] = "southWest";
const char northEast[] = "northEast";
const char southEast[] = "southEast";

const float rightWheelCoeff = 1.034; 

const float wheelbase = 0.147;
const float tol = wheelbase * 0.1;
const float wheelRadius = 0.033;
const float reactionThreshold = 0.22 + 0.06;
const float lidarMinRange = 0.15;

////////////////////////////////// Observations ///////////////////////////////////

/*
* wrapper
*/
struct Point {
	float x = 0;
	float y = 0;
}; // todo: review

class Observation {
public:

	/*
	* enum for type of observation
	*/
	enum class ObsType {invalid, obstacle, target};

	/* 
	* default constructor 
	* (default obs type == invalid)
	*/
	Observation() {}

	/*
	* initialises lidar observation with location and 
	* observation type (default == obstacle)
	*/
	Observation(float x, float y, ObsType t = ObsType::obstacle) {
		setObservation(x, y, t);
	}

	/*
	* sets location and type of obstacle 
	* (default == obstacle)
	*/
	void setObservation(float x, float y, ObsType t = ObsType::obstacle) {
		location.x = x;
		location.y = y; 
		label = t;
	}

	/*
	* getter for observation location
	* @returns location (struct Point)
	*/
	Point getLocation() const {
		assert(isValid());
		return location;
	}

	/*
	* getter for radial distance to observation
	* @returns distance from origin to observation
	*/
	float getDistance() const {
		assert(isValid());
		float a = pow(location.x, 2); 
		float b = pow(location.y, 2);
		return sqrt(a + b);
	}

	/*
	* getter for angle from heading to observation
	* @returns angle in rads (pos left, neg right)
	*/
	float getAngle() const {
		assert(isValid());
		return atan2(location.y, location.x);
	}

	/*
	* getter for observation type
	* @returns observation label 
	*/
	ObsType getLabel() const {
		return label;
	}

	/*
	* invalidates observation when avoided
	*/
	void invalidate() {
		//assert(isValid());
		label = ObsType::invalid;
	}

	/*
	* checks whether observation is invalid
	* @returns true if valid, false otherwise
	*/
	bool isValid() const {
		return ObsType::invalid != label;
	}

private:
	Point location;
	ObsType label = ObsType::invalid;
};

////////////////////////////////// Tasks ///////////////////////////////////////

struct ActionInterface {

	/*
	* callback interface for task controls
	* interface implemented in main thread
	*/
	virtual void executeMotorAction(float speedLeft, float speedRight) = 0;
};

class AbstractTask {
public:

	/* 
	* enum for task success/failure criteria
	*/
	enum ResultCodes { nothing = 0, disturbance_gone = 1, 
	failed = 2, new_disturbance = 3 };

	/*
	* task result helper struct
	*/
	struct TaskResult {
		ResultCodes result = nothing;
		Observation newDisturbance;
		void setDisturbance(Observation d) {
			newDisturbance = d;
			result = new_disturbance;
		}
	};

	/*
	* initialises with previous task variables for chaining
	* called by straight tasks, overriden by avoid tasks
	*/
	virtual void init(shared_ptr<AbstractTask> &t, Observation d) {
		// fixme: review variables
		takeAction = t->takeAction;
		desiredMotorDrive = t->desiredMotorDrive;
		motorDriveLeft = t->desiredMotorDrive;
		motorDriveRight = t->desiredMotorDrive;
		disturbance = d;
	}

	/*
	* pure virtual method for task execution to be 
	* implemented by the child class
	*/
	virtual TaskResult taskExecutionStep(float samplingrate, 
		const vector<Observation>& obs) = 0;

	/*
	* registers actuator interface with target task
	* (otherwise pointer passed in task init method)
	*/
	void registerInterface(ActionInterface* ta) {
		takeAction = ta;
	}

	/*
	* sets the initial/desired motor speed
	*/
	void setInitialSpeed(const float speed) {
		// fixme: review variables
		desiredMotorDrive = speed;
		motorDriveLeft = desiredMotorDrive;
		motorDriveRight = desiredMotorDrive * rightWheelCoeff;
	}

	void resetTaskDuration() {
		taskDuration = 0;
	}

	/*
	* getter for motor linear velocity
	* @returns kinematic linear velocity
	*/
	float getMotorLinearVelocity() {
		// average of both motors converted to m/s
		return (motorDriveLeft+motorDriveRight) / 2.0 * robotDrive2realSpeed;
	}

	/*
	* getter for motor angular velocity
	* @returns kinematic angular velocity
	*/
	float getMotorAngularVelocity() {
		// fixme: doesn't seem quite right
		return (motorDriveLeft - motorDriveRight) / wheelbase * robotDrive2realSpeed;
	}

protected:

	/*
	* event handler for motors
	*/
	void eventNewMotorAction() {
		if (nullptr != takeAction) {
			takeAction->executeMotorAction(motorDriveLeft, motorDriveRight);
		}
	}

	// inhereted for chaining
	ActionInterface* takeAction = nullptr;
	float motorDriveLeft = 0;
	float motorDriveRight = 0;
	float desiredMotorDrive = 0;

	// the disturbance
	Observation disturbance;

	// based on lidar rpm
	float taskDuration = 0.0;

	//float reactionThreshold = 0.2;

	// todo: motor speed conversion
	float robotDrive2realSpeed = 0.1;
};

struct StraightTask : AbstractTask {

	float robotSteering = 0;
	float accSteeringError = 0;
	float accSpeedError = 0;

	float disturbanceLookahead = 4; 

	vector<Observation> prevTrackingObs;

	/*
	* task execution step called by agent event handler
	* handles steering error then disturbance detection
	*/
	virtual TaskResult taskExecutionStep(float samplingrate, 
		const vector<Observation>& obs);
};

template <int signedYawScalar>
struct Rotate90Task : AbstractTask {

	// overrides init method in base class
	virtual void init(shared_ptr<AbstractTask> &t, Observation d) {
		AbstractTask::init(t, d);
		motorDriveLeft = desiredMotorDrive *(float)signedYawScalar;
		motorDriveRight = -desiredMotorDrive *(float)signedYawScalar;
	}

	virtual TaskResult taskExecutionStep(float samplingrate, 
		const vector<Observation>& obs) {
		TaskResult tr;

		float startAngle = 0.0; 
		if (disturbance.isValid()) {
			startAngle = disturbance.getAngle();
		}

		float omega = getMotorAngularVelocity();
		// fixme: numbers don't quite add up hence constant 2
		float angle = startAngle + omega * taskDuration * 2.5;

		if (abs(angle - startAngle) > M_PI/2) {
			tr.result = ResultCodes::disturbance_gone;
			// todo: new motor action -> straight
			// todo: what if prev action was avoid?
			disturbance.invalidate();
			return tr;
		}

		eventNewMotorAction();
		taskDuration += (1/samplingrate);
		return tr;
	}
};

struct Rotate90Left : Rotate90Task<-1> {};
struct Rotate90Right : Rotate90Task<1> {};

////////////////////////////// Planner /////////////////////////////////////////////

struct StateTransition {
	int src;
	int dest;
	int label;
};

class AbstractPlanner {
public:

	virtual vector<shared_ptr<AbstractTask>> eventNewDisturbance(
		const vector<Observation>& obs, const Observation& disturbance) = 0;
};

struct StateMachineLTL : AbstractPlanner {

	int nStates;
	const int init = 0;

	list<int> *graph;
	vector<StateTransition> trans;

	// fixme: invalid obs when increased
	float lateralHorizon = 0.5; // m

	StateMachineLTL(int _nStates) {
		graph = new list<int>[_nStates]();
		nStates = _nStates;
	}

	virtual vector<shared_ptr<AbstractTask>> eventNewDisturbance(
		const vector<Observation>& obs, const Observation& disturbance);

	// consider making const method
	void setTransition(int src, int dest, int label) {
		graph[src].push_back(dest);
        StateTransition st = {src, dest, label};
        trans.push_back(st);
	}

	void setLateralHorizon(float _lateralHorizon) {
		lateralHorizon = _lateralHorizon;
	}

	vector<shared_ptr<AbstractTask>> generatePlan(set<int> accept) {
		vector<int> path = generatePath(accept);
		vector<shared_ptr<AbstractTask>> plan;

		// logging stuff
		vector<const char*> trajectory;

		// get tasks for state transitons
		for (unsigned i = 1; i < path.size(); i++) {
			for (auto& st : trans) {
				if (st.src == path[i-1]
					&& st.dest == path[i]) {
					shared_ptr<AbstractTask> task;
			        switch (st.label) {
			            case 0:
			            	trajectory.push_back("straight");
			                task = make_shared<StraightTask>();
			                break;
			            case 1:
			            	trajectory.push_back("right");
			            	task = make_shared<Rotate90Right>();
			                break;
			            case -1:
			            	trajectory.push_back("left");
			            	task = make_shared<Rotate90Left>();
			                break;
			        }
					plan.push_back(task);
				}
			}
		}

		logMe(accept, path, trajectory);
		return plan;
	}

	vector<int> generatePath(set<int> accept) {
		// performance stats
		vector<int> stackSize;
		vector<int> stackCapacity;
		vector<int> setSize;

		// random stuff
		random_device rd; 
		mt19937 gen(rd()); 
		// set difference
		vector<int> diff;

		vector<int> stack;
		set<int> reachable;
		bool invariant = true;

		stack.push_back(init);
		reachable.insert(init);

		while(!stack.empty() && invariant) {
			int s = stack.back();

			// performance stats
			stackSize.push_back(stack.size());
			stackCapacity.push_back(stack.capacity());
			setSize.push_back(reachable.size());	

			bool subset = includes(reachable.begin(), reachable.end(),
			graph[s].begin(), graph[s].end());

			if (subset) {
				stack.pop_back();
				invariant = accept.find(s) == accept.end();
			} else {
				set_difference(graph[s].begin(), graph[s].end(),
				reachable.begin(), reachable.end(),
				inserter(diff, diff.begin()));

				if (!diff.empty()) {
				    uniform_int_distribution<> distr(0, diff.size()-1);  
					int s_ = diff[distr(gen)];
					diff.clear();

					stack.push_back(s_);
					reachable.insert(s_);
				} 
			}
		}

		saveDfsData(stackSize, sizeof(stack), stackCapacity, 
			setSize, sizeof(reachable));

		if (invariant) {
			// empty path
			stack.clear();
			return stack;
		} else {
			// the path
			return stack;
		}
	}

	void logMe(set<int> accept, vector<int> path, 
		vector<const char*> trajectory) {
		logger.printf("*****************************************\n");

		// log accept states
		logger.printf("ACCEPT: ");
		for (auto& a : accept) {
			logger.printf("%d ", a);
		}
		logger.printf("\n");

		// log path
		logger.printf("PATH: ");
		for (auto&  p: path) {
			logger.printf("%d ", p);
		}
		logger.printf("\n");

		// log plan
		logger.printf("PLAN: ");
		for (unsigned i = 0; i < trajectory.size(); i++) {
			if (i < (trajectory.size()-1)) {
				logger.printf("%s -> ", trajectory[i]);
			} else {
				logger.printf("%s -> default\n", trajectory[i]);
			}
		}

		logger.printf("*****************************************\n");
	}

	void saveObs(const char* name, const vector<Observation>& obs) {
		char tmp[256];
		sprintf(tmp,"../%s/%s%03d.dat", run, name, nEvents);
		//fprintf(stderr,"%s\n",tmp);
		FILE* f = fopen(tmp,"wt");
		fprintf(f,"x\ty\tr\tphi\n");
		for(unsigned i = 0; i < obs.size();i++) {
			if (obs[i].isValid()) {
				fprintf(f,"%4.4f\t%4.4f\t%4.4f\t%4.4f\n",
				obs[i].getLocation().x,
				obs[i].getLocation().y,
				obs[i].getDistance(),
				obs[i].getAngle());
			}
		}
		fclose(f);
	}

	void saveDfsData(const vector<int> stackSize, const int stackMemory, 
		const vector<int> stackCapacity, const vector<int> setSize, const int setMemory) {
		char tmp[256];
		sprintf(tmp,"../%s/dfs%03d.dat", run, nEvents);
		//fprintf(stderr,"%s\n",tmp);
		FILE* f = fopen(tmp,"wt");
		fprintf(f,"STACK:\nn\tcapacity\n");
		for(unsigned i = 0; i < stackSize.size();i++) {
			fprintf(f,"%d\t%d\n",
			stackSize[i], stackCapacity[i]);
		}
		fprintf(f,"COMPILE MEM: %d bytes\n\n", + stackMemory);
		fprintf(f,"SET:\nn\n");
		for(unsigned i = 0; i < setSize.size();i++) {
			fprintf(f,"%d\n", setSize[i]);
		}
		fprintf(f,"COMPILE MEM: %d bytes", + setMemory);
		fprintf(f,"\n\nADJ. LIST COMPILE MEM: %d bytes", sizeof(graph) + sizeof(list<int>) * nStates);
		fprintf(f,"\nSTATE SIZE: %d bytes", sizeof(int));
		fprintf(f,"\nSTATE SPACE SIZE: %d bytes", sizeof(int) * nStates);
		fclose(f);
	}	
};


////////////////////////////// Agent ////////////////////////////////////////////////

class Agent {
public:

	void eventNewRelativeCoordinates(float samplingrate,
		const vector<Observation>& obs);

	void setTargetTask(shared_ptr<AbstractTask> t) {
		targetTask = t;
		currentTask = t;
	}

	void setPlanner(shared_ptr<AbstractPlanner> p) {
		planner = p;
	}

private:
	// default task straight
	shared_ptr<AbstractTask> targetTask;

	// the task we are in
	shared_ptr<AbstractTask> currentTask;

	// the planner
	shared_ptr<AbstractPlanner> planner;	

	// the plan if available
	vector<shared_ptr<AbstractTask>> plan;

	void saveMap(const vector<Observation>& obs) {
		char tmp[256];
		sprintf(tmp,"../%s/map%03d.dat", run, nEvents);
		//fprintf(stderr,"%s\n",tmp);
		FILE* f = fopen(tmp,"wt");
		fprintf(f,"x\ty\tr\tphi\n");
		for(unsigned i = 0; i < obs.size();i++) {
			if (obs[i].isValid()) {
				fprintf(f,"%4.4f\t%4.4f\t%4.4f\t%4.4f\n",
				obs[i].getLocation().x,
				obs[i].getLocation().y,
				obs[i].getDistance(),
				obs[i].getAngle());
			}
		}
		fclose(f);
	}
};


#endif