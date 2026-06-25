#include "command_drone/KeyControl.hpp"
#include "command_drone/CommandDroneNode.hpp"

#include <sys/select.h>
#include <unistd.h>
#include <termios.h>
#include <fcntl.h>

#include <chrono>
#include <thread>
#include <iostream>

KeyControl::KeyControl()
{
    
    m_tty_fd = open("/dev/tty", O_RDONLY | O_NONBLOCK);
    // Save terminal settings
    tcgetattr(m_tty_fd, &m_oldt);
    m_oldf = fcntl(m_tty_fd, F_GETFL, 0);
    
}

KeyControl::~KeyControl()
{
    // Restore settings
    tcsetattr(m_tty_fd, TCSANOW, &m_oldt);
    fcntl(m_tty_fd, F_SETFL, m_oldf);

    m_running = false;
	m_key_thread.join();
}

void KeyControl::TakeContol(OffboardControl& offboardControl)
{
    termios newt;
    newt = m_oldt;
    // Disable line buffering and echo
    newt.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(m_tty_fd, TCSANOW, &newt);   
    // Make stdin non-blocking
    fcntl(m_tty_fd, F_SETFL, m_oldf | O_NONBLOCK);

    m_key_thread = std::thread(&KeyControl::ControlDroneKey, this, std::ref(offboardControl.m_m_trajectory), std::ref(offboardControl.m_pos_trajectory));
}

void KeyControl::ControlDroneKey(std::mutex &posMutex, std::array<float, 3>& pos){
    char ch;
    while(m_running)
    {
        ssize_t n = read(m_tty_fd, &ch, 1);
        if (n > 0) {
            std::lock_guard<std::mutex> lock(posMutex); 
            if (ch == 'z') { 
				pos[2] -= 0.1; 
			}
            if (ch == 's') { 
				pos[2] += 0.1; 
			}
            if (ch == 'q') { 
				break; 
			}
        }
        std::this_thread::sleep_for(10ms);
    }
}