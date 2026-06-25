#ifndef KEY_CONTROL_COMMAND_DRONE
#define KEY_CONTROL_COMMAND_DRONE

// #include "command_drone/CommandDroneNode.hpp"

#include <thread>
#include <mutex>
#include <termios.h>
#include <atomic>

class OffboardControl;

class KeyControl
{
public:
	KeyControl();

	~KeyControl();

	void TakeContol(OffboardControl& offboardControl);

private:

	void ControlDroneKey(std::mutex &posMutex, std::array<float, 3>& pos);

	std::thread m_key_thread;
	std::atomic<bool> m_running{true};

	termios m_oldt;
	int m_oldf;
	int m_tty_fd;
};

#endif  // KEY_CONTROL_COMMAND_DRONE