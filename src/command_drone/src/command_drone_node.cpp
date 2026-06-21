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

#include <sys/select.h>
#include <unistd.h>
#include <termios.h>
#include <fcntl.h>

using namespace std::chrono_literals;

class OffboardControl : public rclcpp::Node
{
public:
	OffboardControl() : Node("offboard_control")
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

		offboard_setpoint_counter_ = 0;
		m_pos_trajectory = {0,0,-2};

		termios newt;
		m_tty_fd = open("/dev/tty", O_RDONLY | O_NONBLOCK);
		// Save terminal settings
		tcgetattr(m_tty_fd, &m_oldt);
		newt = m_oldt;
		// Disable line buffering and echo
		newt.c_lflag &= ~(ICANON | ECHO);
		tcsetattr(m_tty_fd, TCSANOW, &newt);
		// Make stdin non-blocking
		m_oldf = fcntl(m_tty_fd, F_GETFL, 0);
		fcntl(m_tty_fd, F_SETFL, m_oldf | O_NONBLOCK);
		
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

		m_key_thread = std::thread(&OffboardControl::control_drone_key, this);

	}
	~OffboardControl(){
		m_running = false;
		m_key_thread.join();
		// Restore settings
		tcsetattr(m_tty_fd, TCSANOW, &m_oldt);
		fcntl(m_tty_fd, F_SETFL, m_oldf);
		
	}

	void arm();
	void disarm();
	void control_drone_key();

private:
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
	std::thread m_key_thread;
	std::atomic<bool> m_running{true};

	termios m_oldt;
	int m_oldf;
	int m_tty_fd;

	void publish_offboard_control_mode();
	void publish_trajectory_setpoint();
	void publish_vehicle_command(uint16_t command, float param1 = 0.0, float param2 = 0.0);
};

/**
 * @brief Send a command to Arm the vehicle
 */
void OffboardControl::arm()
{
  // needs param set NAV_DLL_ACT 0 to work 
	publish_vehicle_command(px4_msgs::msg::VehicleCommand::VEHICLE_CMD_COMPONENT_ARM_DISARM, 1.0);

	RCLCPP_INFO(this->get_logger(), "Arm command send");
}

/**
 * @brief Send a command to Disarm the vehicle
 */
void OffboardControl::disarm()
{
	publish_vehicle_command(px4_msgs::msg::VehicleCommand::VEHICLE_CMD_COMPONENT_ARM_DISARM, 0.0);

	RCLCPP_INFO(this->get_logger(), "Disarm command send");
}

/**
 * @brief Publish the offboard control mode.
 *        For this example, only position and altitude controls are active.
 */
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

/**
 * @brief Publish a trajectory setpoint
 *        For this example, it sends a trajectory setpoint to make the
 *        vehicle hover at 5 meters with a yaw angle of 180 degrees.
 */
void OffboardControl::publish_trajectory_setpoint()
{
	std::lock_guard<std::mutex> lock(m_m_trajectory); 
	px4_msgs::msg::TrajectorySetpoint msg{};
	msg.position = m_pos_trajectory;
	msg.yaw = -3.14; // [-PI:PI]
	msg.timestamp = this->get_clock()->now().nanoseconds() / 1000;
	trajectory_setpoint_publisher_->publish(msg);
}

/**
 * @brief Publish vehicle commands
 * @param command   Command code (matches px4_msgs::msg::VehicleCommand and MAVLink MAV_CMD codes)
 * @param param1    Command parameter 1
 * @param param2    Command parameter 2
 */
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

void OffboardControl::control_drone_key(){
    char ch;
    while(m_running)
    {
        ssize_t n = read(m_tty_fd, &ch, 1);
        if (n > 0) {
            std::lock_guard<std::mutex> lock(m_m_trajectory); 
            if (ch == 'z') { 
				m_pos_trajectory[2] -= 0.1; 
			}
            if (ch == 's') { 
				m_pos_trajectory[2] += 0.1; 
			}
            if (ch == 'q') { 
				break; 
			}
        }
        std::this_thread::sleep_for(10ms);
    }
}

int main(int argc, char *argv[])
{
	setvbuf(stdout, NULL, _IONBF, BUFSIZ);
	rclcpp::init(argc, argv);
	rclcpp::spin(std::make_shared<OffboardControl>());

	rclcpp::shutdown();
	return 0;
}
