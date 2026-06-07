from launch import LaunchDescription
from launch_ros.actions import Node

def generate_launch_description():
    return LaunchDescription([
        Node(
            package='command_drone',
            namespace='command_drone',
            executable='command_drone_node',
            name='command_drone_node'
        )
    ])