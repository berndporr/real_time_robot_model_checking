#include "agent.h"

int nEvents = 0;
float detectionThreshold = 0;

Logger logger;

bool jobdone = false;

void Agent::eventNewRelativeCoordinates(float samplingrate,
	const vector<Observation>& obs) { 

	AbstractTask::TaskResult tr = currentTask->taskExecutionStep(samplingrate, obs);

	saveMap(obs);

	if (tr.result == 3) {
		logger.printf("%d TASK RESULT = NEW DISTURBANCE!!!!!!\n", nEvents, tr.result);
	} else if (tr.result == 1) {
		logger.printf("%d TASK RESULT = DISTURBANCE ELIMINATED\n", nEvents, tr.result);
	} else {
		logger.printf("%d TASK RESULT = nothing\n", nEvents, tr.result);
	}

	/////////////////////////////////// reactive /////////////////////////////////

	if (onestep) {
		if (tr.result == 3 && tr.newDisturbance.isValid()) {
			logger.printf("x = %f y = %f r = %f phi = %f\n", tr.newDisturbance.getLocation().x, 
					tr.newDisturbance.getLocation().y, tr.newDisturbance.getDistance(),
					tr.newDisturbance.getAngle());
			float angle = tr.newDisturbance.getAngle();	
			if ((angle < 0) && tr.newDisturbance.getDistance() < reactionThreshold) {
				logger.printf("LEFT TURN!!!\n");
				auto left = make_shared<Rotate90Left>();
				left->init(currentTask, tr.newDisturbance);
				currentTask = left;
			} else if (tr.newDisturbance.getDistance() < reactionThreshold) {
				logger.printf("RIGHT TURN!!!\n");
				auto right = make_shared<Rotate90Right>();
				right->init(currentTask, tr.newDisturbance);
				currentTask = right;
			}
		}

		if (tr.result == 1) {
			currentTask = targetTask;
		}
	} else {
		if (tr.result == AbstractTask::disturbance_gone) {
			if (plan.empty()) {
				currentTask = targetTask;
				logger.printf("Plan empty so back to default task...\n");
			} else {
				// launching straight task
				plan[0]->init(currentTask, tr.newDisturbance);
				currentTask = plan[0];
				plan.erase(plan.begin());
				logger.printf("Plan not empty so on to next task...\n");

			}
		}

		if (tr.result == AbstractTask::new_disturbance) {
			if(!plan.empty()) {
				logger.printf("Waiting to execute plan, distance to disturbance = %f\n", tr.newDisturbance.getDistance());
			}
			
			logger.printf("x = %f y = %f r = %f phi = %f\n", tr.newDisturbance.getLocation().x, 
					tr.newDisturbance.getLocation().y, tr.newDisturbance.getDistance(),
					tr.newDisturbance.getAngle());

			if (tr.newDisturbance.isValid() && plan.empty()) {
				logger.printf("Plan empty so creating a new one...\n");
				plan = planner->eventNewDisturbance(obs, tr.newDisturbance);
			}

			if ((tr.newDisturbance.isValid()) 
				&& (tr.newDisturbance.getDistance() <= reactionThreshold)) {
				if (!plan.empty()) {
					// launching avoid task
					plan[0]->init(currentTask, tr.newDisturbance);
					currentTask = plan[0];
					targetTask->resetTaskDuration();
					plan.erase(plan.begin());
					logger.printf("Plan not empty so on to the next task..\n");
				} else {
					logger.printf("Plan empty so stopping...\n");
					exit(1);
				}
			}
		}
	}

	nEvents++;
}

AbstractTask::TaskResult StraightTask::taskExecutionStep(float samplingrate,
	const vector<Observation>& obs) {

	/* step 1: adjust for steering error */
	vector<Point> current;
	vector<Point> previous;

	for(unsigned i = 0; i < obs.size();i++) {
		if (obs[i].isValid()) {
			if (i < prevTrackingObs.size() ) {
				if (prevTrackingObs[i].isValid()) {
					current.push_back(obs[i].getLocation());
					previous.push_back(prevTrackingObs[i].getLocation());
				}
			}
		}
	}

	float dx = 0;
	float dy = 0;

	// todo: could try bining the v
	vector<float> trackingVelocity = {0, 0};
	for (unsigned i = 0; i < current.size(); i++) {
		dx = current[i].x - previous[i].x;
		dy = current[i].y - previous[i].y;

		trackingVelocity[0] += dx / (1/samplingrate);
		trackingVelocity[1] += dy / (1/samplingrate);
	} 

	trackingVelocity[0] = trackingVelocity[0] / current.size();
	trackingVelocity[1] = trackingVelocity[1] / current.size();
	float motorLinearVelocity = getMotorLinearVelocity();

	// fixme: crude error handling 
	if (abs(trackingVelocity[1]) > motorLinearVelocity || 
		abs(trackingVelocity[1]) > abs(trackingVelocity[0])) {
		trackingVelocity[1] = 0;
	}

	prevTrackingObs = obs;

	float steeringError = 0;
	if (abs(trackingVelocity[1]) > 0) {
		// setpoint is zero but need the sign
		steeringError = trackingVelocity[1];	
		accSteeringError += steeringError;

		// fixme: crude tracking cutoff
		if (abs(accSteeringError) > 0.07) {
			accSteeringError = 0;
		}
	} 

	// NOTE Took out PI control as levelled motors to go straight
	//robotSteering = steeringError * 1.0 + accSteeringError * 1.0; 
	//motorDriveLeft = desiredMotorDrive + robotSteering;
	//otorDriveRight = desiredMotorDrive - robotSteering;
	eventNewMotorAction();

	/* step 2: look for disturbance  */
	// fixme: need to adjust linear velocity with error
	detectionThreshold = motorLinearVelocity * disturbanceLookahead;

	Observation disturbance;
	float minx = detectionThreshold;

	// delay to stop things catching 
	// but better if distance dependent 
	// on the motor speed
	if (taskDuration > 0.5) {
		for (unsigned i = 0; i < obs.size(); i++) {
			if ((obs[i].isValid())) {
				Point location = obs[i].getLocation();
				if ((abs(location.x) < detectionThreshold) 
					&& (abs(location.y) <= wheelbase) 
					&& (location.x > lidarMinRange) 
					&& (location.x < minx)) {
					disturbance = obs[i];
					minx = location.x;
				}
			} 
		}
	}

	TaskResult tr;
	if (disturbance.isValid()) {
		accSteeringError = 0;
		tr.setDisturbance(disturbance);
	} 

	if (samplingrate > 0) {
		taskDuration += (1/samplingrate);
	}
	return tr;
}

vector<shared_ptr<AbstractTask>> StateMachineLTL::eventNewDisturbance(
	const vector<Observation>& obs, const Observation& disturbance) {

	auto start = high_resolution_clock::now();

	logger.printf("UPDATING MODEL...\n");
	logger.printf("lateralHorizon = %f reactionThreshold = %f detectionThreshold = %f\n", 
		lateralHorizon, reactionThreshold, detectionThreshold);

	set<int> accept;

 	float northOffset = 0;
 	if (abs(disturbance.getLocation().x) > reactionThreshold) {
 		northOffset = abs(disturbance.getLocation().x - reactionThreshold);
 	}

 	logger.printf("(1) forward sim reaction threshold to disturbance x...\noffset = %f\n", northOffset);
 	logger.printf("(2) checking if obstacles blocking west and east horizon states...\n");

 	// forward simulate obs and filter
 	vector<Observation> westHorizon;
 	vector<Observation> eastHorizon;
 	for (unsigned i = 0; i < obs.size(); i++) {
		if (obs[i].isValid()) {
			Point location = obs[i].getLocation();
			Observation ob(location.x - northOffset, location.y);
			// S_w = {s1, s3}
			if ((ob.getLocation().y < lateralHorizon)
				&& (ob.getLocation().y > 0) // changed from lidar min
				&& (abs(ob.getLocation().x) <= reactionThreshold)) {
					westHorizon.push_back(ob);
			}
			// S_e = {s2, s4}
			if ((ob.getLocation().y > -lateralHorizon)
				&& (ob.getLocation().y < 0) // changed from lidar min
				&& (abs(ob.getLocation().x) <= reactionThreshold)) {
				eastHorizon.push_back(ob);
			}
		} 
	}

	// data collection
	saveObs(west, westHorizon);
	saveObs(east, eastHorizon);

	logger.printf("west size = %d  east size = %d\n", westHorizon.size(), eastHorizon.size());
	logger.printf("(3) calculating the nearest disturbance in west and east directions...\n");

	// nearest west
	Observation nearestWest;
	float miny = lateralHorizon;
	for (unsigned i = 0; i < westHorizon.size(); i++) {
		if (westHorizon[i].isValid()) {
			Point location = westHorizon[i].getLocation();
			if (abs(location.y) < miny 
				//&& abs(location.y) > reactionThreshold
				&& location.x >= -wheelbase) {
				nearestWest = westHorizon[i];
				miny = abs(location.y);
			}
		} 
	}

	if (miny == lateralHorizon) {
		nearestWest.setObservation(0, lateralHorizon);
	}

	// nearest east
	Observation nearestEast;
	miny = lateralHorizon;
	for (unsigned i = 0; i < eastHorizon.size(); i++) {
		if (eastHorizon[i].isValid()) {
			Point location = eastHorizon[i].getLocation();
			if (abs(location.y) < miny
				//&& abs(location.y) > reactionThreshold
				&& location.x >= -wheelbase) {
				nearestEast = eastHorizon[i];
				miny = abs(location.y);
			}
		} 
	}

	if (miny == lateralHorizon) {
		nearestEast.setObservation(0, -lateralHorizon);
	}

	logger.printf("nearest west x = %f y = %f r = %f phi = %f\n", 
		nearestWest.getLocation().x, nearestWest.getLocation().y,
		nearestWest.getDistance(), nearestWest.getAngle());

	logger.printf("nearest east x = %f y = %f r = %f phi = %f\n", 
		nearestEast.getLocation().x, nearestEast.getLocation().y,
		nearestEast.getDistance(), nearestEast.getAngle());

	logger.printf("(4) checking whether agent can move at least %fm in each direction...\n", 0.35);
	bool westDirectionSafe = abs(nearestWest.getLocation().y) > 0.35; 
	bool eastDirectionSafe = abs(nearestEast.getLocation().y) > 0.35;
	logger.printf("west safe = %d  east safe = %d \n", westDirectionSafe, eastDirectionSafe);

	// way is clear either direction S_w U S_e = {s1, s2, s3, s4}
	if ((westHorizon.size() == 0 || abs(nearestWest.getLocation().y) == lateralHorizon)  
		&& (eastHorizon.size() == 0 || abs(nearestEast.getLocation().y) == lateralHorizon)) {
		accept.insert(3);
		accept.insert(4);
		vector<shared_ptr<AbstractTask>> plan = generatePlan(accept);
		auto duration = duration_cast<microseconds>(high_resolution_clock::now() - start);
		logger.printf("MODEL UPDATE COMPLETE!  %f ms\n", duration * 0.001);
		return plan;
	}

	// disturbance left so go right S_e = {s2, s4}
	if ((westHorizon.size() > 0) 
		&& (eastHorizon.size() == 0 || abs(nearestEast.getLocation().y) == lateralHorizon)) {
		accept.insert(4);
		vector<shared_ptr<AbstractTask>> plan = generatePlan(accept);
		auto duration = duration_cast<microseconds>(high_resolution_clock::now() - start);
		logger.printf("MODEL UPDATE COMPLETE!  %f ms\n", duration * 0.001);
		return plan;
	}

	// disturbance right so go left S_w = {s1, s3}
	if ((westHorizon.size() == 0 || abs(nearestWest.getLocation().y) == lateralHorizon) 
		&& (eastHorizon.size() > 0)) {
		accept.insert(3);
		vector<shared_ptr<AbstractTask>> plan = generatePlan(accept);
		auto duration = duration_cast<microseconds>(high_resolution_clock::now() - start);
		logger.printf("MODEL UPDATE COMPLETE!  %f ms\n", duration * 0.001);
		return plan;
	}

	logger.printf("obstacles either direction so planning an extra step\n");
	
	// no room so go south S_s = {s13, s14}
	if (!(westDirectionSafe || eastDirectionSafe)) {
		accept.insert(14);
		vector<shared_ptr<AbstractTask>> plan = generatePlan(accept);
		auto duration = duration_cast<microseconds>(high_resolution_clock::now() - start);
		logger.printf("MODEL UPDATE COMPLETE!  %f ms\n", duration * 0.001);
		return plan;
	}
	logger.printf("enough room to move either side so checking polar states safe\n", westDirectionSafe, eastDirectionSafe);

	// lateral sim offset
	float westOffset = nearestWest.getLocation().y - reactionThreshold;
	float eastOffset = nearestEast.getLocation().y + reactionThreshold;

	logger.printf("(5) lateral sim reaction threshold to nearest distrubance y in each direction...\n");
	logger.printf("west offset = %f east offset = %f\n", westOffset, eastOffset);

	// forward sim and east offset
	vector<Observation> northEastHorizon;
	vector<Observation> southEastHorizon;
	for (unsigned i = 0; i < obs.size(); i++) {
		if ((obs[i].isValid())) {
			Point location = obs[i].getLocation();
			Observation ob(location.x - northOffset, location.y - eastOffset);
			// S_ne = {s6, s8}
			if ((ob.getLocation().x > reactionThreshold) 
				&& (ob.getLocation().x <= reactionThreshold * 2) 
				&& (abs(ob.getLocation().y) <= (wheelbase+tol) * 0.5)) {
				northEastHorizon.push_back(ob);
			}
			// S_se = {s10, s12}
			if ((ob.getLocation().x < -reactionThreshold) 
				&& (ob.getLocation().x >= -reactionThreshold * 2) 
				&& (abs(ob.getLocation().y) <= (wheelbase+tol) * 0.5)) {
				southEastHorizon.push_back(ob);
			}
		} 
	}

	// forward sim and west offset
	vector<Observation> northWestHorizon;
	vector<Observation> southWestHorizon;
	for (unsigned i = 0; i < obs.size(); i++) {
		if ((obs[i].isValid())) {
			Point location = obs[i].getLocation();
			Observation ob(location.x - northOffset, location.y - westOffset);
			// S_nw = {s5, s7}
			if ((ob.getLocation().x > reactionThreshold) 
				&& (ob.getLocation().x <= reactionThreshold * 2) 
				&& (abs(ob.getLocation().y) <= (wheelbase+tol) * 0.5)) {
				northWestHorizon.push_back(ob);
			}
			// S_sw = {s9, s11}
			if ((ob.getLocation().x < -reactionThreshold) 
				&& (ob.getLocation().x >= -(reactionThreshold * 2)) 
				&& (abs(ob.getLocation().y) <= (wheelbase+tol) * 0.5)) {
				southWestHorizon.push_back(ob);
			}
		} 
	}

	// data collection
	saveObs(northEast, northEastHorizon);
	saveObs(southEast, southEastHorizon);
	saveObs(northWest, northWestHorizon);
	saveObs(southWest, southWestHorizon);

	logger.printf("(6) checking if polar horizon states are disturbance free...\n");
	logger.printf("nw size = %d  sw size = %d  ne size = %d  se size = %d\n", northWestHorizon.size(),
		southWestHorizon.size(), northEastHorizon.size(), southEastHorizon.size());

	// east north/south horizon states
	if ((westDirectionSafe && eastDirectionSafe) 
		|| (!westDirectionSafe && eastDirectionSafe)) {
		// S_ne = {s6, s8}
		if (northEastHorizon.size() == 0) {
			accept.insert(8);
		}
		// S_se = {s10, s12}
		if (southEastHorizon.size() == 0) {
			accept.insert(12);
		}
	}

	// west north/south horizon states
	if ((westDirectionSafe && eastDirectionSafe) 
		|| (westDirectionSafe && !eastDirectionSafe)) {
		// S_nw = {s5, s7}
		if (northWestHorizon.size() == 0) {
			accept.insert(7);
		}
		// S_sw = {s9, s11}
		if (southWestHorizon.size() == 0) {
			accept.insert(11);
		}
	}

	vector<shared_ptr<AbstractTask>> plan = generatePlan(accept);
	auto duration = duration_cast<microseconds>(high_resolution_clock::now() - start);
	logger.printf("MODEL UPDATE COMPLETE!  %f ms\n", duration * 0.001);
 	return plan;
}
