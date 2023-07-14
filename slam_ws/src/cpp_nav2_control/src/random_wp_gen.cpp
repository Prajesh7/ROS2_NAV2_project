#include "rclcpp/rclcpp.hpp"
#include "rclcpp_action/rclcpp_action.hpp"
#include "geometry_msgs/msg/pose_stamped.hpp"
#include "geometry_msgs/msg/twist.hpp"
#include "turtlesim/msg/pose.hpp"
#include "nav2_msgs/action/navigate_to_pose.hpp"
#include "nav2_util/geometry_utils.hpp"
#include "geometry_msgs/msg/pose_with_covariance_stamped.hpp"
#include "tf2/LinearMath/Quaternion.h"
#include "std_msgs/msg/string.hpp"
#include <iostream>

using namespace std;

class GoalGenerator: public rclcpp::Node
{
    rclcpp::Publisher<std_msgs::msg::String>::SharedPtr node_news_pub;
    rclcpp::Subscription<geometry_msgs::msg::PoseWithCovarianceStamped>::SharedPtr turtle_pose_sub;
    rclcpp::Publisher<geometry_msgs::msg::PoseWithCovarianceStamped>::SharedPtr init_pose_pub;
    rclcpp::Publisher<geometry_msgs::msg::PoseStamped>::SharedPtr target_pose_pub;
    rclcpp_action::Client<nav2_msgs::action::NavigateToPose>::SharedPtr nav_pose_client;
    rclcpp::TimerBase::SharedPtr timer_;

    double target_dist_;
    int lock_flag = 0;

    public:
        GoalGenerator(): Node("cpp_goal_generator_node")
        {
            RCLCPP_INFO(this->get_logger(), "Goal generator node active");
            init_pose_pub = this->create_publisher<geometry_msgs::msg::PoseWithCovarianceStamped>("initialpose", 10);
            target_pose_pub = this->create_publisher<geometry_msgs::msg::PoseStamped>("goal_pose", 10);
            node_news_pub = this->create_publisher<std_msgs::msg::String>("my_custom_node_news", 10);
            turtle_pose_sub = this->create_subscription<geometry_msgs::msg::PoseWithCovarianceStamped>(
                "amcl_pose", 
                10,
                std::bind(&GoalGenerator::turtle_pose_sub_callback, this, std::placeholders::_1)
                );
            // nav_pose_client = rclcpp_action::create_client<nav2_msgs::action::NavigateToPose>(shared_from_this(),"navigate_to_pose");
            timer_ = this->create_wall_timer(
                std::chrono::seconds(5), 
                std::bind(&GoalGenerator::send_random_goal, this)
                );
            for(int i = 0; i < 3; i ++)
            {
                initial_pose();
                std::this_thread::sleep_for(std::chrono::seconds(1));
            }
        }

        void initial_pose()
        {
            auto  init_pose = geometry_msgs::msg::PoseWithCovarianceStamped();
            init_pose.header.frame_id = "map";
            init_pose.pose.pose.position.x = 0.0;
            init_pose.pose.pose.position.y = 0.0;
            init_pose.pose.pose.position.z = 0.0;
            init_pose.pose.pose.orientation.w = 1.0;

            init_pose_pub->publish(init_pose);

        }
        void turtle_pose_sub_callback(geometry_msgs::msg::PoseWithCovarianceStamped msg)
        {
            // RCLCPP_INFO(this->get_logger(), "temp msg");
            double curr_x = msg.pose.pose.position.x;
            double curr_y = msg.pose.pose.position.y;

            double curr_dist = std::sqrt(curr_x * curr_x + curr_y * curr_y);
            double dist_delta = std::abs(target_dist_ - curr_dist);
            auto news = std_msgs::msg::String();
            // char message[30]; 
            // std::sprintf(message, "Distance delta: %f", dist_delta);
            // news.data = message;
            // node_news_pub->publish(news);

            if(dist_delta > 0.1)
            {
                lock_flag = 1;
            }
            else
            {
                lock_flag = 0;
            }
        }

        void send_random_goal()
        {
            // RCLCPP_INFO(this->get_logger(), "temp msg");
            double x_min, x_max, y_min, y_max, theta_min, theta_max;
            x_min = -3.0;
            x_max = 10.0;
            y_min = -2.0;
            y_max = 5.0;
            theta_min = -3.1415926535;
            theta_max = 3.1415926535;

            // Generate a random target goal pose within the given bounds
            double random_x = x_min + (std::rand() / (RAND_MAX / (x_max - x_min)));
            double random_y = y_min + (std::rand() / (RAND_MAX / (y_max - y_min)));
            double random_theta = theta_min + (std::rand() / (RAND_MAX / (theta_max - theta_min)));

            auto news = std_msgs::msg::String();

            if(lock_flag == 0)
            {
                target_dist_ = std::sqrt(random_x * random_x + random_y * random_y);
                tf2::Quaternion quat;
                quat.setRPY(0.0, 0.0, random_theta);
                auto goal = geometry_msgs::msg::PoseStamped();
                goal.header.frame_id = "map";
                goal.pose.position.x = random_x;
                goal.pose.position.y = random_y;
                goal.pose.orientation.x = quat.getX();
                goal.pose.orientation.y = quat.getY();
                goal.pose.orientation.z = quat.getZ();
                goal.pose.orientation.w = quat.getW();
                target_pose_pub->publish(goal);
                
                char message[100]; // Assuming a maximum of 100 characters for the message
                std::sprintf(message, "new target: x-> %f, y-> %f, theta-> %f", random_x, random_y, random_theta);
                news.data = message;
                node_news_pub->publish(news);

                RCLCPP_WARN(this->get_logger(), "new target: x-> %f, y-> %f, theta-> %f", random_x, random_y, random_theta);
            }
            else
            {
                news.data = "waiting for bot to reach target";
                node_news_pub->publish(news);
                RCLCPP_WARN(this->get_logger(), "waiting for bot to reach target");
            }
        }
};

int main(int argc, char **argv)
{
    rclcpp::init(argc, argv);
    auto node = std::make_shared<GoalGenerator>();
    rclcpp::spin(node);
    rclcpp::shutdown();
    return 0;
}
