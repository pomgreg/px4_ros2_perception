#!/bin/bash

# 1. Cleanup
killall -9 ruby gz server uxagent 2>/dev/null
pkill -f px4 2>/dev/null

# # 2. Start XRCE-DDS Agent
# cmd.exe /c start wsl.exe -e bash -ic "uxagent udp4 -p 8888; exec bash"

# # 3. Start Drone 0 (Chaser/Radar) at origin
# cmd.exe /c start wsl.exe -e bash -ic "cd ~/PX4-Autopilot; PX4_SYS_AUTOSTART=4001 PX4_GZ_MODEL=x500 ./build/px4_sitl_default/bin/px4 -i 0; exec bash"

# sleep 1 # Wait for Gazebo to actually open

# # 4. Start Drone 1 (Target) at 20,20
# cmd.exe /c start wsl.exe -e bash -ic "cd ~/PX4-Autopilot; PX4_SYS_AUTOSTART=4001 PX4_GZ_STANDALONE=1 PX4_GZ_MODEL_POSE='20,20,0' PX4_GZ_MODEL=x500 ./build/px4_sitl_default/bin/px4 -i 1; exec bash"

# # 5. Start the Radar Bridge (Path B)
# cmd.exe /c start wsl.exe -e bash -ic "ros2 run ros_gz_bridge parameter_bridge /world/default/model/x500/link/base_link/sensor/radar_sensor/scan@sensor_msgs/msg/LaserScan[gz.msgs.LaserScan; exec bash"