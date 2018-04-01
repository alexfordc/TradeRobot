#if !defined(ROBOT_FACTORY_H)
#define ROBOT_FACTORY_H

#include "rapidjson/document.h"
#include "DualTrust.h"
#include "BollBreak.h"
#include "SingleMA.h"
#include "Turtle.h"
#include "MABreak.h"

namespace RobotFactory {
	Robot* create(const char* robot, int id, const rapidjson::Value& params) {
		Robot* pRobot = nullptr;

		if (strcmp(robot, "DualTrust") == 0) {
			pRobot = new DualTrust(params, id);
		}
		else if (strcmp(robot, "BollBreak") == 0) {
			pRobot = new BollBreak(params, id);
		}
		else if (strcmp(robot, "SingleMA") == 0) {
			pRobot = new SingleMA(params, id);
		}
		else if (strcmp(robot, "Turtle") == 0) {
			pRobot = new Turtle(params, id);
		}
		else if (strcmp(robot, "MABreak") == 0) {
			pRobot = new MABreak(params, id);
		}

		return pRobot;
	}
}

#endif
