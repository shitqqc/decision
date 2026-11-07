cmake_minimum_required(VERSION 3.16)
project(rm_referee)

if(NOT CMAKE_CXX_STANDARD)
  set(CMAKE_CXX_STANDARD 17)
  set(CMAKE_CXX_STANDARD_REQUIRED ON)
endif()

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(std_msgs REQUIRED)
find_package(geometry_msgs REQUIRED)
find_package(rm_referee_msgs REQUIRED)

# ---------------------------------------------------------------------------
# Serial transport (non-template part of the driver)
# ---------------------------------------------------------------------------
add_library(rm_referee_serial
  src/serial_port.cpp
)
target_include_directories(rm_referee_serial PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include/${PROJECT_NAME}>
)

target_compile_features(rm_referee_serial PUBLIC cxx_std_17)

# ---------------------------------------------------------------------------
# Executables: one per role, each a thin main over the role subclass
# ---------------------------------------------------------------------------
add_executable(infantry_referee_node src/infantry_node.cpp)
target_link_libraries(infantry_referee_node rm_referee_serial)
ament_target_dependencies(infantry_referee_node
  rclcpp std_msgs geometry_msgs rm_referee_msgs)

add_executable(sentry_referee_node src/sentry_node.cpp)
target_link_libraries(sentry_referee_node rm_referee_serial)
ament_target_dependencies(sentry_referee_node
  rclcpp std_msgs geometry_msgs rm_referee_msgs)

add_executable(radar_referee_node src/radar_node.cpp)
target_link_libraries(radar_referee_node rm_referee_serial)
ament_target_dependencies(radar_referee_node
  rclcpp std_msgs geometry_msgs rm_referee_msgs)

# Diagnostic node for bring-up: listens to 0x0201 only.
add_executable(robot_status_test_node src/robot_status_test_node.cpp)
target_link_libraries(robot_status_test_node rm_referee_serial)
ament_target_dependencies(robot_status_test_node
  rclcpp std_msgs geometry_msgs rm_referee_msgs)

# ---------------------------------------------------------------------------
# Install
# ---------------------------------------------------------------------------
install(TARGETS rm_referee_serial
  EXPORT export_rm_referee
  ARCHIVE DESTINATION lib
  LIBRARY DESTINATION lib
  RUNTIME DESTINATION bin
)
install(TARGETS
  infantry_referee_node
  sentry_referee_node
  radar_referee_node
  robot_status_test_node
  DESTINATION lib/${PROJECT_NAME}
)
install(DIRECTORY include/${PROJECT_NAME}/
  DESTINATION include/${PROJECT_NAME}
)
install(DIRECTORY launch config
  DESTINATION share/${PROJECT_NAME}
)
install(FILES README.md
  DESTINATION share/${PROJECT_NAME}
)

ament_export_targets(export_rm_referee HAS_LIBRARY_TARGET)
ament_export_dependencies(rclcpp std_msgs geometry_msgs rm_referee_msgs)

# ---------------------------------------------------------------------------
# Tests
# ---------------------------------------------------------------------------
if(BUILD_TESTING)
  find_package(ament_cmake_gtest REQUIRED)

  ament_add_gtest(test_protocol test/test_protocol.cpp)
  target_link_libraries(test_protocol rm_referee_serial)
  ament_target_dependencies(test_protocol
    rclcpp std_msgs geometry_msgs rm_referee_msgs)

  ament_add_gtest(test_referee_node test/test_referee_node.cpp)
  target_link_libraries(test_referee_node rm_referee_serial)
  ament_target_dependencies(test_referee_node
    rclcpp std_msgs geometry_msgs rm_referee_msgs)
endif()

ament_package()
