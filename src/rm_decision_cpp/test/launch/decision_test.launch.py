import os

from ament_index_python.packages import get_package_share_directory
from launch import LaunchDescription
from launch_ros.actions import Node


def generate_launch_description():
    pkg_share = get_package_share_directory('rm_decision_cpp')
    config = os.path.join(pkg_share, 'test', 'config', 'decision_test.yaml')
    return LaunchDescription([
        Node(
            package="rm_decision_cpp",
            executable="decision_test_node.py",
            parameters=[{"data_file": config}],
            output="screen"
        )
    ])
