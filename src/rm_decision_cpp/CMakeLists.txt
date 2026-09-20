cmake_minimum_required(VERSION 3.11)
project(rm_decision_cpp)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

find_package(ament_cmake REQUIRED)
find_package(ament_index_cpp REQUIRED)
find_package(rclcpp REQUIRED)
find_package(rclcpp_action REQUIRED)
find_package(decision_interfaces REQUIRED)
find_package(nav_msgs REQUIRED)
find_package(nav2_msgs REQUIRED)
find_package(geometry_msgs REQUIRED)
find_package(behaviortree_cpp REQUIRED)
find_package(std_msgs REQUIRED)
find_package(action_msgs REQUIRED)
find_package(tf2 REQUIRED)
find_package(yaml-cpp REQUIRED)

include_directories(include)

set(ros_dependencies
  ament_index_cpp
  rclcpp
  rclcpp_action
  decision_interfaces
  nav_msgs
  nav2_msgs
  geometry_msgs
  behaviortree_cpp
  std_msgs
  action_msgs
  tf2
)

set(DECISION_SOURCES
  src/infrastructure/game_info_referee_ingress.cpp
  src/infrastructure/odom_pose_ingress.cpp
  src/infrastructure/sentry_cmd_egress.cpp
  src/infrastructure/bt_snapshot_publisher.cpp
  src/infrastructure/bt_status_latch.cpp
  src/application/bt_engine.cpp
  src/application/decision_app.cpp
  src/domain/zone_loader.cpp
  src/bt/action/auto_actions.cpp
  src/bt/action/nav_action.cpp
  src/bt/action/tactical_action.cpp
  src/bt/action/resource_actions.cpp
  src/bt/action/change_stance_action.cpp
  src/bt/action/gimbal_action.cpp
  src/bt/action/recovery_actions.cpp
  src/bt/condition/auto_conditions.cpp
  src/bt/condition/tactical_condition.cpp
  src/bt/condition/resource_conditions.cpp
  src/bt/condition/change_stance_condition.cpp
  src/bt/condition/gimbal_condition.cpp
)

add_library(${PROJECT_NAME} SHARED ${DECISION_SOURCES})
target_include_directories(${PROJECT_NAME} PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>
)
ament_target_dependencies(${PROJECT_NAME} ${ros_dependencies})
target_link_libraries(${PROJECT_NAME} yaml-cpp)

add_executable(tree_exec_node src/tree_exec_node.cpp)
target_link_libraries(tree_exec_node ${PROJECT_NAME})
ament_target_dependencies(tree_exec_node ${ros_dependencies})

install(TARGETS ${PROJECT_NAME} tree_exec_node
  ARCHIVE DESTINATION lib
  LIBRARY DESTINATION lib
  RUNTIME DESTINATION lib/${PROJECT_NAME}
)

install(DIRECTORY include/ DESTINATION include)
install(DIRECTORY tree config launch web DESTINATION share/${PROJECT_NAME})
install(PROGRAMS
  scripts/bt_monitor_web.py
  scripts/mock_decision_pub.py
  DESTINATION lib/${PROJECT_NAME}
)

if(BUILD_TESTING)
  find_package(ament_lint_auto REQUIRED)
  set(ament_cmake_copyright_FOUND TRUE)
  set(ament_cmake_cpplint_FOUND TRUE)
  ament_lint_auto_find_test_dependencies()
endif()

ament_package()
