cmake_minimum_required(VERSION 3.8)
project(cpp_nav2_control)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# find dependencies
find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(std_msgs REQUIRED)
find_package(std_srvs REQUIRED)
find_package(geometry_msgs REQUIRED)
find_package(nav2_msgs REQUIRED)
find_package(nav2_util REQUIRED)
find_package(turtlesim REQUIRED)

add_executable(random_goal_generator src/random_wp_gen.cpp)
ament_target_dependencies(random_goal_generator rclcpp std_msgs std_srvs geometry_msgs nav2_msgs nav2_util turtlesim)

install(TARGETS 
random_goal_generator
  DESTINATION lib/${PROJECT_NAME}
)

ament_package()
