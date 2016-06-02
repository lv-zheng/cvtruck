#include "stdafx.h"

#include "truck.hpp"

#include <cassert>
#include <string>

#include <boost/asio/io_service.hpp>
#include <boost/asio/read_until.hpp>
#include <boost/asio/serial_port.hpp>
#include <boost/asio/streambuf.hpp>
#include <boost/asio/write.hpp>
#include <boost/system/system_error.hpp>

namespace lvzheng {

class truck_impl : public truck {
public:
	truck_impl(const std::string&);
	void go(direction, strength) override;
	void stop() override;
	~truck_impl();

private:
	boost::asio::io_service io;
	boost::asio::serial_port serial;

	int get_straight_strength_value(strength);
	int get_turn_strength_value(strength);
	void send_and_get_reply(const std::string&);
};

truck_impl::truck_impl(const std::string& device) try:
	serial(io, device)
{
	serial.set_option(boost::asio::serial_port::baud_rate(9600));
} catch (boost::system::system_error&) {
	throw connection_error("device not found");
}

void truck_impl::go(truck::direction dir, truck::strength stngth)
{
	std::string command;
	int strengthv;

	switch (dir) {
	case direction::FORWARD:
		command = "FORWARD";
		strengthv = get_straight_strength_value(stngth);
		break;
	case direction::BACKWARD:
		command = "BACKWARD";
		strengthv = get_straight_strength_value(stngth);
		break;
	case direction::LEFT:
		command = "LEFT";
		strengthv = get_turn_strength_value(stngth);
		break;
	case direction::RIGHT:
		command = "RIGHT";
		strengthv = get_turn_strength_value(stngth);
		break;
	default:
		assert(0);
	}

	command.push_back(' ');
	command += std::to_string(strengthv);
	command.push_back('\n');

	send_and_get_reply(command);
}

void truck_impl::stop()
{
	send_and_get_reply("STOP\n");
}

int truck_impl::get_straight_strength_value(strength stngth)
{
	switch (stngth) {
	case strength::LIGHT:
		return 72;
	case strength::MEDIUM:
		return 128;
	case strength::FIERCE:
		return 255;
	default:
		assert(0);
	}
}

int truck_impl::get_turn_strength_value(strength stngth)
{
	switch (stngth) {
	case strength::LIGHT:
		return 112;;
	case strength::MEDIUM:
		return 128;
	case strength::FIERCE:
		return 144;
	default:
		assert(0);
	}
}

void truck_impl::send_and_get_reply(const std::string& blob)
{
	boost::system::error_code ec;

	write(serial, boost::asio::buffer(blob), ec);
	if (ec)
		throw connection_error("fail to send");

	boost::asio::streambuf sb;
	read_until(serial, sb, '\n', ec);

	if (ec)
		throw connection_error("fail to recv");

	auto bufs = sb.data();
	std::string got{boost::asio::buffers_begin(bufs),
		boost::asio::buffers_end(bufs)};

	if (got.size() != 3 || got != "OK\n") {
		throw connection_error("truck report error");
	}
}

truck_impl::~truck_impl() = default;

std::unique_ptr<truck> create_truck(const std::string& device)
{
	return std::make_unique<truck_impl>(device);
}

} // namespace lvzheng
