//this code for blue_detect to control arm 

#include <vector>
#include <iostream>
#include <stdint.h>
#include <cstdlib>
#include "armDriver.h"
#include "rosReciever.h"
#include "ros/ros.h"
#include "unistd.h"

static ArmDriver *p_armDriver = NULL;

void chatterCallback(const std_msgs::Float32MultiArray::ConstPtr& msg)
{
   // ROS_INFO("I heard: [%d]", msg->data);
    int i = 0;
    float com[4];
    for(std::vector<float>::const_iterator it = msg->data.begin(); it != msg->data.end(); ++it)
    {
        com[i] = *it;
        i++;
    }

        if(com[0] == 100){              
           p_armDriver->moveMode(com[1],com[2],20,3);    
            sleep(1);
           p_armDriver->moveMode(com[1],com[2],com[3],3); 
         }
        else if(com[0]==200){
           p_armDriver->moveMode(com[1],com[2],com[3],6);
        }   
        else if ( com[0] >=0 && com[0] <= 10){
            p_armDriver->changeArmStatus(LINEAR, (Action)com[0], 100);
        }   
        else if(com[0] < 0){ 
           com[0] = -com[0];
           p_armDriver->changeArmStatus(AXIS, (Action)com[0], 100); 
        }   
  
}

int main(int argc, char * args[]){
       
     if (argc < 2 ){
        cerr <<"You must specify the serial port name"<<endl;
        cerr <<"example : ./main /dev/ttyACM0"<<endl;
        return 0;
    }
    p_armDriver = new ArmDriver(args[1]);
    p_armDriver->connectToArm();
    ROSreciver(argc,args);//, chatterCallback);

    return 0;
}
