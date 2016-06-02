#ifndef LVZHENG_TRUCK_HPP
#define LVZHENG_TRUCK_HPP


#include <memory>
#include <stdexcept>
#include <iostream>
#include <string.h>

namespace lvzheng {

class truck {
public:
	class connection_error : public std::runtime_error {
		using runtime_error::runtime_error;
	};
	enum class direction {
		FORWARD = 0,
		BACKWARD,
		LEFT,
		RIGHT,
	};
	enum class strength {
		LIGHT = 0,
		MEDIUM,
		FIERCE,
	};
	virtual void go(direction, strength) = 0;
	virtual void stop() = 0;
	virtual ~truck() = default;

protected:
	truck() = default;
};

std::unique_ptr<truck> create_truck(const std::string& device);

} // namespace lvzheng

#endif // LVZHENG_TRUCK_HPP
