add_executable(test_stuff test_stuff.cpp)
target_link_libraries(test_stuff framework glfw glut ${GLFW_LIBRARIES})
install(TARGETS test_stuff DESTINATION bin)

add_executable(kinect_client kinect_client.cpp)
target_link_libraries(kinect_client framework glfw glut ${GLFW_LIBRARIES})
install(TARGETS kinect_client DESTINATION bin)

add_executable(calib_inverter calib_inverter.cpp)
target_link_libraries(calib_inverter framework glfw glut ${GLFW_LIBRARIES})
install(TARGETS calib_inverter DESTINATION bin)