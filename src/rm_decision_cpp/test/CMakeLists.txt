find_package(Python3 REQUIRED COMPONENTS Interpreter)

set(TEST_REGISTRATION "${CMAKE_CURRENT_BINARY_DIR}/bt_test_registration.cpp")
add_custom_command(
  OUTPUT "${TEST_REGISTRATION}"
  COMMAND "${Python3_EXECUTABLE}" "${CMAKE_CURRENT_SOURCE_DIR}/generate_registration.py"
    "${PROJECT_SOURCE_DIR}/src/application/bt_engine.cpp" "${TEST_REGISTRATION}"
  DEPENDS generate_registration.py "${PROJECT_SOURCE_DIR}/src/application/bt_engine.cpp"
  VERBATIM
)

add_executable(bt_test_node bt_test_node.cpp "${TEST_REGISTRATION}")
target_compile_features(bt_test_node PRIVATE cxx_std_17)
ament_target_dependencies(bt_test_node ${ros_dependencies})
target_link_libraries(bt_test_node ${PROJECT_NAME})

install(TARGETS bt_test_node
  RUNTIME DESTINATION lib/${PROJECT_NAME}
)
install(PROGRAMS
  scripts/bt_monitor_web.py
  scripts/decision_test_node.py
  DESTINATION lib/${PROJECT_NAME}
)
install(FILES scripts/decision_test_web.py
  DESTINATION lib/${PROJECT_NAME}
)
install(DIRECTORY launch/ DESTINATION share/${PROJECT_NAME}/launch)
install(DIRECTORY config web tree DESTINATION share/${PROJECT_NAME}/test)
