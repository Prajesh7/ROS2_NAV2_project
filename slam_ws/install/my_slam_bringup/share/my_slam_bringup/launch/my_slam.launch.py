#!/usr/bin/env python3
import os

from ament_index_python.packages import get_package_share_directory
from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument
from launch.actions import IncludeLaunchDescription
from launch.launch_description_sources import PythonLaunchDescriptionSource
from launch.substitutions import LaunchConfiguration
from launch_ros.actions import Node
import launch.logging 

logger = launch.logging.get_logger()
logger.setLevel(launch.logging.logging.WARNING)

TURTLEBOT3_MODEL = os.environ['TURTLEBOT3_MODEL']
print(TURTLEBOT3_MODEL)
def generate_launch_description():
    launch_file_dir = os.path.join(get_package_share_directory('turtlebot3_gazebo'), 'launch')
    pkg_gazebo_ros = get_package_share_directory('gazebo_ros')
    
    use_sim_time = LaunchConfiguration('use_sim_time', default='true')
    x_pose = LaunchConfiguration('x_pose', default='-2.0')
    y_pose = LaunchConfiguration('y_pose', default='-4.0')

    world = os.path.join(
        get_package_share_directory('turtlebot3_gazebo'),
        'worlds',
        'my_world.world'
    )
    print(world)
    gzserver_cmd = IncludeLaunchDescription(
        PythonLaunchDescriptionSource(
            os.path.join(pkg_gazebo_ros, 'launch', 'gzserver.launch.py')
        ),
        launch_arguments={'world': world}.items()
    )

    gzclient_cmd = IncludeLaunchDescription(
        PythonLaunchDescriptionSource(
            os.path.join(pkg_gazebo_ros, 'launch', 'gzclient.launch.py')
        )
    )

    robot_state_publisher_cmd = IncludeLaunchDescription(
        PythonLaunchDescriptionSource(
            os.path.join(launch_file_dir, 'robot_state_publisher.launch.py')
        ),
        launch_arguments={'use_sim_time': use_sim_time}.items()
    )

    spawn_turtlebot_cmd = IncludeLaunchDescription(
        PythonLaunchDescriptionSource(
            os.path.join(launch_file_dir, 'spawn_turtlebot3.launch.py')
        ),
        launch_arguments={
            'x_pose': x_pose,
            'y_pose': y_pose
        }.items()
    )
    # -----------nav-------------------------

    # map_dir = LaunchConfiguration(
    #     'map',
    #     default=os.path.join(
    #         get_package_share_directory('turtlebot3_navigation2'),
    #         'map',
    #         'map.yaml'))
    map_dir = LaunchConfiguration(
        'map',
        default='/home/prajesh/maps/my_house.yaml'
    )
    param_file_name = TURTLEBOT3_MODEL + '.yaml'
    param_dir = LaunchConfiguration(
        'params_file',
        default=os.path.join(
            get_package_share_directory('turtlebot3_navigation2'),
            'param',
            param_file_name))

    nav2_launch_file_dir = os.path.join(get_package_share_directory('nav2_bringup'), 'launch')

    rviz_config_dir = os.path.join(
        get_package_share_directory('nav2_bringup'),
        'rviz',
        'nav2_default_view.rviz')
    
    d1 = DeclareLaunchArgument(
        'map',
        default_value=map_dir,
        description='Full path to map file to load')

    d2 = DeclareLaunchArgument(
        'params_file',
        default_value=param_dir,
        description='Full path to param file to load')

    d3 = DeclareLaunchArgument(
        'use_sim_time',
        default_value='false',
        description='Use simulation (Gazebo) clock if true')

    # i1 = IncludeLaunchDescription(
    #     PythonLaunchDescriptionSource([nav2_launch_file_dir, '/bringup_launch.py']),
    #     launch_arguments={
    #         'map': map_dir,
    #         'use_sim_time': use_sim_time,
    #         'params_file': param_dir
    #         }.items(),
    # )

    i1 = IncludeLaunchDescription(
        PythonLaunchDescriptionSource([
            os.path.join(nav2_launch_file_dir, 'bringup_launch.py')
        ]),
        launch_arguments={
            'map': map_dir,
            'use_sim_time': use_sim_time,
            'params_file': param_dir
            }.items(),
    )

    n1 = Node(
        package='rviz2',
        executable='rviz2',
        name='rviz2',
        arguments=['-d', rviz_config_dir],
        parameters=[{'use_sim_time': use_sim_time}],
        output='screen')

    n2 = Node(
        package='cpp_nav2_control',
        executable='random_goal_generator'
    )

    ld = LaunchDescription([
        d1,
        d2,
        d3,
        i1,
        n1
    ])
    # Add the commands to the launch description
    
    ld.add_action(gzserver_cmd)
    ld.add_action(gzclient_cmd)
    ld.add_action(robot_state_publisher_cmd)
    ld.add_action(spawn_turtlebot_cmd)
    ld.add_action(n2)
    

    return ld