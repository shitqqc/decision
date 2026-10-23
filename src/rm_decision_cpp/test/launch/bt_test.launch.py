import os

from ament_index_python.packages import get_package_share_directory
from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument, IncludeLaunchDescription
from launch.launch_description_sources import PythonLaunchDescriptionSource
from launch.substitutions import LaunchConfiguration
from launch_ros.actions import Node


def generate_launch_description():
    share = get_package_share_directory("rm_decision_cpp")
    return LaunchDescription([
        DeclareLaunchArgument(
            "data_file",
            default_value=os.path.join(share, "test", "config", "decision_test.yaml"),
        ),
        DeclareLaunchArgument(
            "tree_xml_file",
            default_value="test_tree.xml",
        ),
        DeclareLaunchArgument("host", default_value="127.0.0.1"),
        DeclareLaunchArgument("port", default_value="8766"),
        Node(
            package="rm_decision_cpp",
            executable="bt_test_node",
            parameters=[{
                "tree_xml_file": LaunchConfiguration("tree_xml_file"),
            }],
            output="screen",
        ),
        Node(
            package="rm_decision_cpp",
            executable="decision_test_node.py",
            parameters=[{"data_file": LaunchConfiguration("data_file")}],
            output="screen",
        ),
        IncludeLaunchDescription(
            PythonLaunchDescriptionSource(os.path.join(share, "launch", "bt_monitor.launch.py")),
            launch_arguments={
                "host": LaunchConfiguration("host"),
                "port": LaunchConfiguration("port"),
            }.items(),
        ),
    ])
