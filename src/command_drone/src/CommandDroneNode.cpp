#include "command_drone/CommandDroneNode.hpp"


OffboardControl::OffboardControl() : Node("offboard_control")
{
	RCLCPP_INFO(
		this->get_logger(),
		"---Starting offboard control node...---");

	// publishers
	offboard_control_mode_publisher_ = this->create_publisher<px4_msgs::msg::OffboardControlMode>("/fmu/in/offboard_control_mode", 10);
	trajectory_setpoint_publisher_ = this->create_publisher<px4_msgs::msg::TrajectorySetpoint>("/fmu/in/trajectory_setpoint", 10);
	vehicle_command_publisher_ = this->create_publisher<px4_msgs::msg::VehicleCommand>("/fmu/in/vehicle_command", 10);

	// RCLCPP_INFO(this->get_logger(), "pre_flight_checks_pass = %d", m_pre_flight_checks_pass.pre_flight_checks_pass);

	// subcriber
	rclcpp::QoS qos(rclcpp::KeepLast(10));
	qos.best_effort();
	qos.durability_volatile();

	vehicle_status_subscriber_ = this->create_subscription<px4_msgs::msg::VehicleStatus>(
		"/fmu/out/vehicle_status_v1", qos,
		[this](const px4_msgs::msg::VehicleStatus & msg) {
			m_pre_flight_checks_pass = msg;
		});

	m_controlType = Type::KEYBOARD_CONTROL;
	offboard_setpoint_counter_ = 0;
	m_pos_trajectory = {0,0,-2};
	
	timer_ = this->create_wall_timer(100ms, [this]() {
		if (m_pre_flight_checks_pass.pre_flight_checks_pass == true)
		{

			publish_offboard_control_mode();
			publish_trajectory_setpoint();

			if (offboard_setpoint_counter_ == 10) {
				this->arm();
			}


			if (offboard_setpoint_counter_ == 15) {
				this->publish_vehicle_command(
					px4_msgs::msg::VehicleCommand::VEHICLE_CMD_DO_SET_MODE, 1, 6);
			}


			if (offboard_setpoint_counter_ <= 15) {
				offboard_setpoint_counter_++;
			}	
		}
	});

	m_keyControl.TakeContol(*this);
}


void OffboardControl::arm()
{
  // needs param set NAV_DLL_ACT 0 to work 
	publish_vehicle_command(px4_msgs::msg::VehicleCommand::VEHICLE_CMD_COMPONENT_ARM_DISARM, 1.0);

	RCLCPP_INFO(this->get_logger(), "Arm command send");
}


void OffboardControl::disarm()
{
	publish_vehicle_command(px4_msgs::msg::VehicleCommand::VEHICLE_CMD_COMPONENT_ARM_DISARM, 0.0);

	RCLCPP_INFO(this->get_logger(), "Disarm command send");
}

void OffboardControl::publish_offboard_control_mode()
{
	px4_msgs::msg::OffboardControlMode msg{};
	msg.position = true;
	msg.velocity = false;
	msg.acceleration = false;
	msg.attitude = false;
	msg.body_rate = false;
	msg.timestamp = this->get_clock()->now().nanoseconds() / 1000;
	offboard_control_mode_publisher_->publish(msg);
}

void OffboardControl::publish_trajectory_setpoint()
{
	std::lock_guard<std::mutex> lock(m_m_trajectory); 
	px4_msgs::msg::TrajectorySetpoint msg{};
	msg.position = m_pos_trajectory;
	msg.yaw = -3.14; // [-PI:PI]
	msg.timestamp = this->get_clock()->now().nanoseconds() / 1000;
	trajectory_setpoint_publisher_->publish(msg);
}

void OffboardControl::publish_vehicle_command(uint16_t command, float param1, float param2)
{
	px4_msgs::msg::VehicleCommand msg{};
	msg.param1 = param1;
	msg.param2 = param2;
	msg.command = command;
	msg.target_system = 1;
	msg.target_component = 1;
	msg.source_system = 1;
	msg.source_component = 1;
	msg.from_external = true;
	msg.timestamp = this->get_clock()->now().nanoseconds() / 1000;
	vehicle_command_publisher_->publish(msg);
}




