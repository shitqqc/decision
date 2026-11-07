cmake_minimum_required(VERSION 3.16)
project(rm_referee_msgs)

if(NOT CMAKE_CXX_STANDARD)
  set(CMAKE_CXX_STANDARD 17)
  set(CMAKE_CXX_STANDARD_REQUIRED ON)
endif()

find_package(ament_cmake REQUIRED)
find_package(std_msgs REQUIRED)
find_package(geometry_msgs REQUIRED)
find_package(rosidl_default_generators REQUIRED)

set(msg_files
  "msg/DartClientCmd.msg"
  "msg/DartInfo.msg"
  "msg/FieldEvent.msg"
  "msg/GameResult.msg"
  "msg/GameRobotHP.msg"
  "msg/GameStatus.msg"
  "msg/GroundRobotPosition.msg"
  "msg/MapCommand.msg"
  "msg/MapData.msg"
  "msg/MapRobotData.msg"
  "msg/PowerHeat.msg"
  "msg/ProjectileAllowance.msg"
  "msg/RadarInfo.msg"
  "msg/RadarMark.msg"
  "msg/RefereeFrame.msg"
  "msg/RefereeWarning.msg"
  "msg/RfidStatus.msg"
  "msg/RobotBuff.msg"
  "msg/RobotHurt.msg"
  "msg/RobotInteraction.msg"
  "msg/RobotPosition.msg"
  "msg/RobotStatus.msg"
  "msg/SentryInfo.msg"
  "msg/Shoot.msg"
)

rosidl_generate_interfaces(${PROJECT_NAME}
  ${msg_files}
  DEPENDENCIES std_msgs geometry_msgs
)

ament_export_dependencies(rosidl_default_runtime)

install(FILES README.md DESTINATION share/${PROJECT_NAME})

ament_package()
