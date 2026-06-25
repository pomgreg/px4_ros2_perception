#ifndef COMMAND_DRONE_NODE_COMMAND_DRONE
#define COMMAND_DRONE_NODE_COMMAND_DRONE

#include <px4_msgs/msg/offboard_control_mode.hpp>
#include <px4_msgs/msg/trajectory_setpoint.hpp>
#include <px4_msgs/msg/vehicle_command.hpp>
#include <px4_msgs/msg/vehicle_control_mode.hpp>
#include <px4_msgs/msg/vehicle_status.hpp>
#include <rclcpp/rclcpp.hpp>
#include <stdint.h>

#include <chrono>
#include <thread>
#include <iostream>

#include "command_drone/KeyControl.hpp"

// can do get space key to test readin, then get angle and posiion and adjust with zqsdae keys
// drone up scan env to check if other drone then track

using namespace std::chrono_literals;

class OffboardControl : public rclcpp::Node
{
public:
    enum class Type {
        CHASE,
        KEYBOARD_CONTROL,
        MAPPING
    };

	OffboardControl();

	/**
	 * @brief Send a command to Arm the vehicle
	 */
	void arm();

	/**
	 * @brief Send a command to Disarm the vehicle
	 */
	void disarm();

private:
    friend class KeyControl;

	/**
	 * @brief Publish the offboard control mode.
	 *        For this example, only position and altitude controls are active.
	 */
	void publish_offboard_control_mode();

	/**
	 * @brief Publish a trajectory setpoint
	 *        For this example, it sends a trajectory setpoint to make the
	 *        vehicle hover at 5 meters with a yaw angle of 180 degrees.
	 */
	void publish_trajectory_setpoint();

	/**
	 * @brief Publish vehicle commands
	 * @param command   Command code (matches px4_msgs::msg::VehicleCommand and MAVLink MAV_CMD codes)
	 * @param param1    Command parameter 1
	 * @param param2    Command parameter 2
	 */
	void publish_vehicle_command(uint16_t command, float param1 = 0.0, float param2 = 0.0);	

    
	rclcpp::TimerBase::SharedPtr timer_;

	rclcpp::Publisher<px4_msgs::msg::OffboardControlMode>::SharedPtr offboard_control_mode_publisher_;
	rclcpp::Publisher<px4_msgs::msg::TrajectorySetpoint>::SharedPtr trajectory_setpoint_publisher_;
	rclcpp::Publisher<px4_msgs::msg::VehicleCommand>::SharedPtr vehicle_command_publisher_;

	rclcpp::Subscription<px4_msgs::msg::VehicleStatus>::SharedPtr vehicle_status_subscriber_;

	std::atomic<uint64_t> timestamp_;   //!< common synced timestamped

	uint64_t offboard_setpoint_counter_;   //!< counter for the number of setpoints sent

	px4_msgs::msg::VehicleStatus m_pre_flight_checks_pass;

	std::array<float, 3> m_pos_trajectory;
	std::mutex m_m_trajectory;

	Type m_controlType;
	KeyControl m_keyControl;
};

#endif  // COMMAND_DRONE_NODE_COMMAND_DRONE