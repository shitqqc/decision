# Copyright (c) 2018 Intel Corporation
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

"""
Example for spawning multiple robots in Gazebo.

This is an example on how to create a launch file for spawning multiple robots into Gazebo
and launch multiple instances of the navigation stack, each controlling one robot.
The robots co-exist on a shared environment and are controlled by independent nav stacks.
"""

import os.path

from pathlib import Path

from ament_index_python.packages import get_package_prefix, get_package_share_directory

from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument, GroupAction, OpaqueFunction, SetEnvironmentVariable
from launch.substitutions import LaunchConfiguration
from launch_ros.actions import Node, PushRosNamespace, SetRemap


def _check_decision_interfaces_typesupport(_context):
    """Fail fast when interface typesupport libraries are missing (avoids obscure RCL allocator errors)."""
    import os

    try:
        prefix = Path(get_package_prefix('decision_interfaces'))
    except LookupError as exc:
        raise RuntimeError(
            'Package decision_interfaces is not on AMENT_PREFIX_PATH. '
            'Build it with the workspace (e.g. colcon build --packages-up-to rm_decision_cpp) '
            'and source install/setup.bash before launching.'
        ) from exc
    libdir = prefix / 'lib'
    libs = sorted(libdir.glob('libdecision_interfaces__rosidl_typesupport_*_cpp.so'))
    if not libs:
        raise RuntimeError(
            f'No decision_interfaces C++ typesupport libraries under {libdir}. '
            'Rebuild package decision_interfaces in this workspace and source install/setup.bash.'
        )
    rmw = os.environ.get('RMW_IMPLEMENTATION', '').lower()
    if 'cyclonedds' in rmw or 'rmw_cyclonedds' in rmw:
        need_token = 'cyclonedds'
    else:
        # Humble default and typical rmw_fastrtps_cpp installs
        need_token = 'fastrtps'
    if not any(need_token in p.name for p in libs):
        raise RuntimeError(
            f'RMW_IMPLEMENTATION={os.environ.get("RMW_IMPLEMENTATION", "")!r} expects typesupport '
            f'containing {need_token!r}, but under {libdir} only: {[p.name for p in libs]}. '
            'Rebuild decision_interfaces with the same ROS distro/RWM stack, or adjust RMW_IMPLEMENTATION.'
        )
    return []


def create_node(context):
    pkg_share = get_package_share_directory('rm_decision_cpp')
    config = os.path.join(pkg_share, 'config', 'node_params.yaml')
    tree_xml_file = context.perform_substitution(LaunchConfiguration('tree_xml_file', default=''))
    namespace = context.perform_substitution(LaunchConfiguration('namespace', default=''))
    use_fastdds = LaunchConfiguration('use_fastdds_dynamic_history', default='true')
    use_fastdds_val = context.perform_substitution(use_fastdds) == 'true'
    actions = []
    if use_fastdds_val:
        fastdds_xml = os.path.join(pkg_share, 'config', 'fastdds_dynamic_history.xml')
        actions.append(SetEnvironmentVariable('FASTRTPS_DEFAULT_PROFILES_FILE', fastdds_xml))
        actions.append(SetEnvironmentVariable('RMW_FASTRTPS_USE_QOS_FROM_XML', '1'))
    params = [config]
    # Multi-tree XMLs are loaded from share/rm_decision_cpp/tree/; tree_xml_file launch arg is ignored.
    node = Node(
        package='rm_decision_cpp',
        name='tree_exec',
        executable='tree_exec_node',
        parameters=params,
        output='screen',
    )
    if namespace.strip():
        actions.append(
            GroupAction(
                actions=[
                    PushRosNamespace(namespace=namespace),
                    SetRemap('/tf', 'tf'),
                    SetRemap('/tf_static', 'tf_static'),
                    node,
                ]
            )
        )
    else:
        actions.append(node)
    return actions

def generate_launch_description():
    ld = LaunchDescription([
        DeclareLaunchArgument(
            'tree_xml_file',
            default_value='',
            description='Override: full path to tree XML (empty = use node_params.yaml)'),
        DeclareLaunchArgument(
            'use_fastdds_dynamic_history',
            default_value='true',
            description='Use Fast DDS DYNAMIC history to avoid RTPS_READER_HISTORY payload size errors (23 vs 24 bytes)'),
        DeclareLaunchArgument(
            'namespace',
            default_value='nav',
            description='与导航一致（reality 默认 nav）；空字符串则使用全局 /tf'),
        OpaqueFunction(function=_check_decision_interfaces_typesupport),
        OpaqueFunction(function=create_node),
    ])
    return ld