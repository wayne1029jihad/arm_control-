#ifndef talk.h
#define talk.h
#include "std_msgs/Int16.h"
void chatterCallback(const std_msgs::Int16::ConstPtr& msg)
 {
   ROS_INFO("I heard: [%d]", msg->data);
 }



#endif
