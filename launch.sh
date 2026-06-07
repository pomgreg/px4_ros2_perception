colcon build
source install/setup.bash
killall -9 ruby gz server uxagent 2>/dev/null
pkill -f px4 2>/dev/null
ros2 launch my_bringup launch.py