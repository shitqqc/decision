from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument
from launch.substitutions import LaunchConfiguration
from launch_ros.actions import Node


def generate_launch_description():
    return LaunchDescription(
        [
            DeclareLaunchArgument(
                "scenario",
                default_value="cycle",
                description="normal|low_ammo|low_hp|engaged|attack_fort|manual|dead|supply|cycle",
            ),
            DeclareLaunchArgument(
                "period",
                default_value="8.0",
                description="Seconds per scenario when scenario=cycle",
            ),
            DeclareLaunchArgument(
                "ns",
                default_value="/nav",
                description="Namespace prefix for game_info topic",
            ),
            Node(
                package="rm_decision_cpp",
                executable="mock_decision_pub.py",
                name="mock_decision_pub",
                output="screen",
                arguments=[
                    "--scenario",
                    LaunchConfiguration("scenario"),
                    "--period",
                    LaunchConfiguration("period"),
                    "--ns",
                    LaunchConfiguration("ns"),
                ],
            ),
        ]
    )
