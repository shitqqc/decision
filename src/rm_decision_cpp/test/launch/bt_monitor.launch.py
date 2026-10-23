from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument
from launch.substitutions import LaunchConfiguration
from launch_ros.actions import Node


def generate_launch_description():
    return LaunchDescription(
        [
            DeclareLaunchArgument(
                "topic",
                default_value="/nav/bt_snapshot",
                description="BT snapshot JSON topic",
            ),
            DeclareLaunchArgument(
                "port",
                default_value="8765",
                description="HTTP port for the monitor UI",
            ),
            DeclareLaunchArgument(
                "host",
                default_value="0.0.0.0",
                description="HTTP bind address",
            ),
            Node(
                package="rm_decision_cpp",
                executable="bt_monitor_web.py",
                name="bt_monitor_web",
                output="screen",
                arguments=[
                    "--topic",
                    LaunchConfiguration("topic"),
                    "--port",
                    LaunchConfiguration("port"),
                    "--host",
                    LaunchConfiguration("host"),
                ],
            ),
        ]
    )
