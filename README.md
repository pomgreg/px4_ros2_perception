## Depedency
- ROS 2 Humble
- PX4-Autopilot (cloned and compiled)
- ros-humble-ros-gzharmonic (apt install)
- px4_msgs (cloned in src/) #TBD : git submodule add https://github.com/PX4/px4_msgs.git src/px4_msgs

## Installation
1. git clone ce repo
2. Cp px4_patches/4001_gz_x500 in PX4-Autopilot/ROMFS/.../airframes/
3. Cp px4_patches/x500_model.sdf in PX4-Autopilot/Tools/simulation/gz/models/x500/model.sdf
4. colcon build
5. source install/setup.bash

## Launch
./launch.sh