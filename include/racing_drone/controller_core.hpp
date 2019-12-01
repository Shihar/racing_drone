#pragma once

#include <algorithm>
#include <vector>
#include <ros/ros.h>
#include <ros/time.h>
#include <geometry_msgs/Twist.h>
#include <geometry_msgs/Point.h>
#include <nav_msgs/Odometry.h>
#include <tf/transform_datatypes.h>
#include "MPC.hpp"

class Controller
{
public:
	ros::Publisher cmdPublisher; //Control Input publisher
	ros::Subscriber refSubscriber; // Reference Input subscriber
	ros::Subscriber odomSubscriber; // Odometery subscriber
	ros::NodeHandle nh; // Node handle
	int rate; //Control loop rate, Hz 
	int N; //horizon_window
	std::vector<double> P; // Ccst penalties, 9*1 vector
	MPC quad_mpc;	// Model Predictive Controller Object

	geometry_msgs::Twist controlInput; // Control Input message (Twist on /cmd_vel)
	std::vector<double> currState; //x,y,z,vx,vy,vz,roll,pitch,yaw
	geometry_msgs::Pose refPose; // Refernce pose
	
	Controller(int rt, int n, std::vector<double> p, double dt_); //Controller Constructor
	~Controller(); //Controller Destructor
	
	void odomCallback(const nav_msgs::Odometry::ConstPtr& odom); //Odometry Callbcak for updating current state
	void refCallback(const geometry_msgs::Pose::ConstPtr& refPose); // Reference Callback for updating reference Pose
	void computeControlInput(void); // Compute Control Input by MPC optimization
	void publishControlInput(void); // Publish Control Input 
};
