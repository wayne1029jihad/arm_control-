#ifndef _ROS_reciver
#define _ROS_reciver

#include "std_msgs/String.h"
#include "ros/ros.h"
#include "std_msgs/Int16.h"
#include "std_msgs/Int32MultiArray.h"
  
//typedef void (*chatterCallback) (const std_msgs::Int16::ConstPtr& msg);
void chatterCallback(const std_msgs::Int32MultiArray::ConstPtr& msg);
void ROSreciver(int argc , char **argv)
    {
        ros::init(argc ,argv, "reciver");
        ros::NodeHandle n;
        ros::Subscriber sub = n.subscribe("chatter", 1000, chatterCallback);
        ros::spin();
    }






#endif
