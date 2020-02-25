/**
 * @file localizer.cpp
 * @author Swapneel Naphade (naphadeswapneel@gmail.com)
 * @brief Localizes the drone on the map using gate measurements
 * @version 0.1
 * @date 01-15-2020
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

#include <racing_drone/localizer_core.hpp>

int main(int argc, char** argv)
{
    // Setup ROS
    ros::init(argc, argv, "localizer");
    ros::NodeHandle nh("");

    // Load parameters
    double measGain;
    std::vector<double> gatesVec;
    std::string odomSubTopic;
    std::string odomPubTopic;
    std::string gatePoseSubTopic;

    ros::param::get("localizer/measGain", measGain);
    ros::param::get("localizer/gates", gatesVec);
    ros::param::get("localizer/odomSubTopic", odomSubTopic);
    ros::param::get("localizer/odomPubTopic", odomPubTopic);
    ros::param::get("localizer/gatePoseSubTopic", gatePoseSubTopic);

    std::vector<racing_drone::DroneState> gates;
    for(int i=0; i < gatesVec.size(); i+=7 )
    {
        racing_drone::DroneState gateState;
        gateState.position.x = gatesVec[i];
        gateState.position.y = gatesVec[i+1];
        gateState.position.z = gatesVec[i+2];
        gateState.velocity.x = gatesVec[i+3];
        gateState.velocity.y = gatesVec[i+4];
        gateState.velocity.z = gatesVec[i+5];
        gateState.yaw        = gatesVec[i+6]; 
        gates.push_back(gateState);
    }

    Localizer localizer(nh, odomSubTopic, odomPubTopic, gatePoseSubTopic, gates, measGain);
    GateVisual gateVis(nh, gates);

    //Wait for other nodes to initialize
    ros::Rate sleepRate(1.0);
	sleepRate.sleep();
    
    ros::MultiThreadedSpinner multiSpinner(6);
    ros::spin(multiSpinner);

    return 0;
}