/**
 * @file gate_detector_core.hpp
 * @author Swapneel Naphade (naphadeswapneel@gmail.com)
 * @brief gate_detector_core declaration
 * @version 0.1
 * @date 01-05-2020
 * 
 *  Copyright (c) 2020 Swapneel Naphade
 * 
 *  Permission is hereby granted, free of charge, to any person obtaining a copy
 *  of this software and associated documentation files (the "Software"), to deal
 *  in the Software without restriction, including without limitation the rights
 *  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 *  copies of the Software, and to permit persons to whom the Software is
 *  furnished to do so, subject to the following conditions:
 * 
 *  The above copyright notice and this permission notice shall be included in all
 *  copies or substantial portions of the Software.
 * 
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 *  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 *  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 *  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 *  SOFTWARE.
 */

#pragma once

#include <cstdlib>
#include <ros/ros.h>
#include <ros/time.h>
#include <geometry_msgs/Pose.h>
#include <visualization_msgs/Marker.h>
#include <nav_msgs/Odometry.h>
#include <sensor_msgs/Image.h>
#include <sensor_msgs/Imu.h>
#include <std_msgs/Bool.h>
#include <image_transport/image_transport.h>
#include <cv_bridge/cv_bridge.h>
#include <sensor_msgs/image_encodings.h>
#include "racing_drone/GateDetector.hpp"
#include "racing_drone/KalmanFilter.hpp"
#include "racing_drone/MovAvgFilter.hpp"

namespace ublas = boost::numeric::ublas;

class GateDetectorCore
{
    public:
        ros::NodeHandle nh;
        ros::Publisher rawGatePosePub;
        ros::Publisher filteredGatePosePub;
        ros::Publisher successPub;
        ros::Publisher gateImagePub;
        ros::Publisher rawMarkerPub;
        ros::Publisher filtMarkerPub;
        ros::Subscriber odomSub;
        ros::Subscriber imageSub;
        ros::Subscriber imuSub;
        ros::Timer EKF_timer;

        std::string rawGatePubTopic;
        std::string filteredGatePubTopic;
        std::string odomSubTopic;
        std::string imageOutTopic;
        std::string imuTopic;

        ublas::vector<double> uEKF;
        ublas::vector<double> yEKF;
        std::vector<double> wEKF;
        std::vector<double> droneQ;

        GateDetector gd;
        ExtendedKalmanFilter gateEKF;
        MovAvgFilter gateMAF;
        geometry_msgs::Pose rawGatePose; 
        geometry_msgs::Pose filteredGatePose; 
        std_msgs::Bool success;

        cv_bridge::CvImagePtr cv_ptr;

        visualization_msgs::Marker rawGateMarker;
        visualization_msgs::Marker filtGateMarker;

        GateDetectorCore(ros::NodeHandle &node_handle, GateDetector gd_, ExtendedKalmanFilter gateEKF_,
             std::string rawGatePubTopic_, std::string filteredGatePubTopic_, std::string odomSubTopic_,
             std::string imageOutTopic_, std::string imuTopic_);

        ~GateDetectorCore();

        void imageCallback(const sensor_msgs::Image::ConstPtr& ros_img);
        void odomCallback(const nav_msgs::Odometry::ConstPtr& odom); 
        void imuCallback(const sensor_msgs::Imu::ConstPtr& imu);
        void EKF_timerCallback(const ros::TimerEvent& timerEvent);
        
        void updateGatePose(Mat& img); 
        void initMarkers(void);
};

