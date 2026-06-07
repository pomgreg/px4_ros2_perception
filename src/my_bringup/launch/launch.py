from launch import LaunchDescription
from launch.actions import ExecuteProcess, TimerAction, IncludeLaunchDescription
from launch_ros.actions import Node
from launch.launch_description_sources import PythonLaunchDescriptionSource
from ament_index_python.packages import get_package_share_directory
import os

def generate_launch_description():
    command_launch_path = os.path.join(
        get_package_share_directory('command_drone'),
        'launch',
        'launch.py'
    )

    command_launch = IncludeLaunchDescription(
        PythonLaunchDescriptionSource(command_launch_path),
        # launch_arguments={
        #     'use_sim_time': 'true'
        # }.items()
    )
    return LaunchDescription([

        # XRCE-DDS Agent
        ExecuteProcess(
            cmd=[
                'wsl.exe',
                '-e', 'bash', '-ic',
                'uxagent udp4 -p 8888; exec bash'
            ],
            output='screen'
        ),

        # Drone 1
        ExecuteProcess(
            cmd=['./build/px4_sitl_default/bin/px4',
                 '-i', '0'],
            cwd='/home/greg/PX4-Autopilot',
            additional_env={
                'PX4_SYS_AUTOSTART' : '4001',
                'PX4_GZ_MODEL': 'x500',
                'PX4_UDP_PORT': '14540',
                'PX4_GZ_WORLD': "walls"
                # 'NAV_DLL_ACT': '0',
                # 'COM_RC_IN_MODE': '1',
            },
            output='screen'
        ),

        # Drone 2
        ExecuteProcess(
            cmd=['./build/px4_sitl_default/bin/px4',
                 '-i', '1'],
            cwd='/home/greg/PX4-Autopilot',
            additional_env={
                'PX4_SYS_AUTOSTART' : '4001',
                'PX4_GZ_STANDALONE' : '1',
                'PX4_GZ_MODEL_POSE' : '20,20,0',
                'PX4_GZ_MODEL': 'x500',
                'PX4_UDP_PORT': '14541',
                'PX4_GZ_WORLD': "walls"
            },
            output='screen'
        ),

        Node(
            package='ros_gz_bridge',
            executable='parameter_bridge',
            arguments=[
                '/world/walls/model/x500_1/link/radar_link/sensor/radar_sensor/scan/points'
                '@sensor_msgs/msg/PointCloud2'
                '@gz.msgs.PointCloudPacked',
            ],
            output='screen'
        ),
        command_launch

    ])