"""Launch the referee driver for one robot role.

The role selects which executable (and therefore which compile-time command
list) is started; all roles share the node name `referee_node`, so the same
YAML parameter block applies to each of them.

    ros2 launch rm_referee referee.launch.py robot_type:=sentry port:=/dev/ttyUSB0
"""

import os

from ament_index_python.packages import get_package_share_directory
from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument
from launch.substitutions import LaunchConfiguration, PythonExpression
from launch_ros.actions import Node
from launch_ros.parameter_descriptions import ParameterValue


def generate_launch_description():
    config = os.path.join(
        get_package_share_directory('rm_referee'), 'config', 'referee.yaml'
    )

    robot_type = LaunchConfiguration('robot_type')
    port = LaunchConfiguration('port')
    baudrate = LaunchConfiguration('baudrate')

    return LaunchDescription([
        DeclareLaunchArgument(
            'robot_type',
            default_value='infantry',
            choices=['infantry', 'sentry', 'radar'],
            description='Robot role; selects the compile-time command list.',
        ),
        DeclareLaunchArgument(
            'port',
            default_value='/dev/ttyUSB0',
            description='TTL adapter device connected to the referee system.',
        ),
        DeclareLaunchArgument(
            'baudrate',
            default_value='115200',
            description='115200 for the power-management port, 921600 for the video link.',
        ),
        Node(
            package='rm_referee',
            executable=PythonExpression(
                ["'", robot_type, "' + '_referee_node'"]
            ),
            name='referee_node',
            output='screen',
            parameters=[
                config,
                {
                    'port': port,
                    'baudrate': ParameterValue(baudrate, value_type=int),
                },
            ],
        ),
    ])
