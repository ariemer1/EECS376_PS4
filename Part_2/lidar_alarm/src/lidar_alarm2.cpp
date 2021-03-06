// wsn example program to illustrate LIDAR processing.  1/23/15

#include <ros/ros.h> //Must include this for all ROS cpp projects
#include <sensor_msgs/LaserScan.h>
#include <std_msgs/Float32.h> //Including the Float32 class from std_msgs
#include <std_msgs/Bool.h> // boolean message
#include <std_srvs/Trigger.h>

const double MIN_SAFE_DISTANCE = 1.0; // set alarm if anything is within 0.5m of the front of robot

// these values to be set within the laser callback
float ping_dist_in_front_=3.0; // global var to hold length of a SINGLE LIDAR ping--in front
int ping_index_= -1; // NOT real; callback will have to find this
double angle_min_=0.0;
double angle_max_=0.0;
double angle_increment_=0.0;
double range_min_ = 0.0;
double range_max_ = 0.0;
bool laser_alarm_=false;
bool prev_laser_alarm_ = false;

ros::ServiceClient estop_client_;
std_srvs::Trigger estop_srv_;

ros::ServiceClient clear_estop_client_;
std_srvs::Trigger clear_estop_srv_;
// really, do NOT want to depend on a single ping.  Should consider a subset of pings
// to improve reliability and avoid false alarms or failure to see an obstacle
//this function takes in a series of pings and if any of them are less than the minimum safe distance, publish danger
void blockedPath(const sensor_msgs::LaserScan& laser_scan, double rectWidth, double rectHeight) {
    double rightRad    = angle_max_-(3.1415/2);
    double toprightRad = rightRad + atan(rectHeight/rectWidth);
    double topleftRad  = rightRad + 3.1415 - atan(rectHeight/rectWidth);
    double leftRad     = rightRad + 3.1415;
    int indexRight    = (int)(rightRad/angle_increment_);
    int indexTopRight = (int)(toprightRad/angle_increment_);
    int indexTopLeft  = (int)(topleftRad/angle_increment_);
    int indexLeft     = (int)(leftRad/angle_increment_);
    double desiredDist = 0.0;
    double currDist = 0.0;
    double theta = 0.0;
    bool pathBlocked = false; //assume the path is clear
        for(int i = indexRight; i <= indexLeft;i++) {
          currDist = laser_scan.ranges[i];
          theta = (i-indexRight)*angle_increment_; //With the positive x-axis being 0

          if((i >= indexRight && i < indexTopRight) || (i >= indexTopLeft && i <= indexLeft)) {
              desiredDist = sqrt(pow(tan(theta)*rectWidth,2) + pow(rectWidth,2)); //y = rectWidth or the line y = -rectWidth
          } else if(i >= indexTopRight && i < indexTopLeft) {
              desiredDist = sqrt(pow(rectHeight/tan(theta),2) + pow(rectHeight,2));//x = rectHeight
          }

          if(currDist <= desiredDist) { //path is blocked if lidar says we are closer than the rectangle
              pathBlocked = true;
              break;
          }
    }

    laser_alarm_ = pathBlocked;
}

void laserCallback(const sensor_msgs::LaserScan& laser_scan) {
    if (ping_index_<0)  {
	    //for first message received, set up the desired index of LIDAR range to eval
        angle_min_ = laser_scan.angle_min;
        angle_max_ = laser_scan.angle_max;
		angle_increment_ = laser_scan.angle_increment;
        range_min_ = laser_scan.range_min;
        range_max_ = laser_scan.range_max;
        // what is the index of the ping that is straight ahead?
        // BETTER would be to use transforms, which would reference how the LIDAR is mounted;
        // but this will do for simple illustration
        ping_index_ = (int) ((0.0 -angle_min_)/angle_increment_);
        ROS_INFO("LIDAR setup: ping_index = %d",ping_index_);
    }

    blockedPath(laser_scan, 0.5, 2.0);
    if (laser_alarm_ && !prev_laser_alarm_) { //send out warning if there is danger
        ROS_WARN("DANGER ZONE!!");
        estop_client_.call(estop_srv_);
    } else if(prev_laser_alarm_ && !laser_alarm_) {
        clear_estop_client_.call(clear_estop_srv_);
    }
    prev_laser_alarm_ = laser_alarm_;
}


int main(int argc, char **argv) {
    ros::init(argc, argv, "lidar_alarm"); //name this node
    ros::NodeHandle nh;
	//create a Subscriber object and have it subscribe to the lidar topic
    ros::Subscriber lidar_subscriber = nh.subscribe("/scan", 1, laserCallback);
    estop_client_ = nh.serviceClient<std_srvs::Trigger>("estop_service");
    clear_estop_client_ = nh.serviceClient<std_srvs::Trigger>("clear_estop_service");
    ros::spin(); //this is essentially a "while(1)" statement, except it
    // forces refreshing wakeups upon new data arrival
    // main program essentially hangs here, but it must stay alive to keep the callback function alive
    return 0; // should never get here, unless roscore dies
}
