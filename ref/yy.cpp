#include "stdafx.h"
#include <iostream>
#include <string.h>
#include "truck.hpp"

int maintt(int argc, char **argv)
{
	using namespace lvzheng;

	if (argc != 2) {
		std::cerr << "Usage: " << argv[0] << " <serial-port>" << std::endl;
		return 1;
	}

	auto truck = create_truck(argv[1]);
	std::cout << "READY" << std::endl;
	std::string s;
	while (1) {
		std::cout << "truck> ";
		if (s == "w")
			truck->go(truck::direction::FORWARD, truck::strength::LIGHT);
		else if (s == "s")
			truck->go(truck::direction::BACKWARD, truck::strength::LIGHT);
		else if (s == "a")
			truck->go(truck::direction::LEFT, truck::strength::LIGHT);
		else if (s == "d")
			truck->go(truck::direction::RIGHT, truck::strength::LIGHT);
		else if (s == "")
			truck->stop();
		else if (s == "q") {
			truck->stop();
			break;
		}
		else {
			std::cout << "Unknown command" << std::endl;
		}
	}

	return 0;
}
