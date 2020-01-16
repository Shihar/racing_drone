/**
 * @file localizer_core.cpp
 * @author Swapneel Naphade (naphadeswapneel@gmail.com)
 * @brief localizer core definition
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

Localizer::Localizer(ros::NodeHandle nh_, std::string odomSubTopic_, std::string odomPubTopic_, std::string gatePoseTopic_,
              double measGain_)
              : nh(nh_), odomSubTopic(odomSubTopic_), odomPubTopic(odomPubTopic_),
                gatePoseTopic(gatePoseTopic_), measGain(measGain_)
{}

void Localizer::odomSubCallback(const nav_msgs::Odometry::ConstPtr& odomIn)
{
    inOdom = *odomIn;
}

void Localizer::gatePoseSubCallback(const geometry_msgs::Pose::ConstPtr& gtPose) 
{
    gatePoseDrone = *gtPose;
    // gatePoseDrone.position = oRd * gatePose.position;

    std::vector<double> gatePosDrone(3), droneQ(4);
    droneQ[0] = inOdom.pose.pose.orientation.x;
    droneQ[1] = inOdom.pose.pose.orientation.y;
    droneQ[2] = inOdom.pose.pose.orientation.z;
    droneQ[3] = inOdom.pose.pose.orientation.w;

    gatePosDrone[0] = gatePoseDrone.position.x;
    gatePosDrone[1] = gatePoseDrone.position.y;
    gatePosDrone[2] = gatePoseDrone.position.z;

    gatePosDrone = rotateVec(droneQ, gatePosDrone);

    gatePoseDrone.position.x = gatePosDrone[0];
    gatePoseDrone.position.y = gatePosDrone[1];
    gatePoseDrone.position.z = gatePosDrone[2]; // Converted to gate position wrt drone in inertial frame from drone frame.


    projectGatePose();
    findGate();
    publishDroneState();
}

void Localizer::findGate(void)
{
    int minDistIndex = 0;
    double minDist = 10000;

    for(int i=0; i < gates.size(); i++)
    {
        double dist = getGateDistance(gates[i]);
        if( dist < minDist)
        {
            minDist = dist;
            minDistIndex = i;
        }
    }

    currentGateIndex = minDistIndex;

    ROS_INFO( "[localizer] current gate index: %d", currentGateIndex );

    std::vector<double> gateRPY(3), gateQ(4);
    gatePoseOrigin.position.x = gates[currentGateIndex].position.x;
    gatePoseOrigin.position.y = gates[currentGateIndex].position.y;
    gatePoseOrigin.position.z = gates[currentGateIndex].position.z;

    gateRPY[0] = 0.0;
    gateRPY[1] = 0.0;
    gateRPY[2] = gates[currentGateIndex].yaw;

    rpy2quat(gateRPY, gateQ);

    gatePoseOrigin.orientation.x = gateQ[0];
    gatePoseOrigin.orientation.y = gateQ[1];
    gatePoseOrigin.orientation.z = gateQ[2];
    gatePoseOrigin.orientation.w = gateQ[3];


}

/**
 * @brief Projects measured gate pose in drone frame to inertial frame using drone's current pose
 *        { r_go = r_do + o_R_d * r_gd ; q_go = q_gd (x) q_do}
 * 
 */
void Localizer::projectGatePose(void)
{
    // gate = odomIn.pose.pose.position + oRd * gatePose.position;
    std::vector<double> gateQDrone(4), gateQOrigin(4), droneQ(4);
    droneQ[0] = inOdom.pose.pose.orientation.x;
    droneQ[1] = inOdom.pose.pose.orientation.y;
    droneQ[2] = inOdom.pose.pose.orientation.z;
    droneQ[3] = inOdom.pose.pose.orientation.w;

    gateQDrone[0] = gatePoseDrone.orientation.x;
    gateQDrone[1] = gatePoseDrone.orientation.y;
    gateQDrone[2] = gatePoseDrone.orientation.z;
    gateQDrone[3] = gatePoseDrone.orientation.w;

    gateQOrigin = quatMultiply(gateQDrone, droneQ);

    gatePoseOrigin.position.x = inOdom.pose.pose.position.x + gatePoseDrone.position.x;
    gatePoseOrigin.position.y = inOdom.pose.pose.position.y + gatePoseDrone.position.y;
    gatePoseOrigin.position.z = inOdom.pose.pose.position.z + gatePoseDrone.position.z;
    gatePoseOrigin.orientation.x = gateQOrigin[0];
    gatePoseOrigin.orientation.y = gateQOrigin[1];
    gatePoseOrigin.orientation.z = gateQOrigin[2];
    gatePoseOrigin.orientation.w = gateQOrigin[3];

}

double Localizer::getGateDistance(racing_drone::DroneState gateState)
{
    return std::sqrt( (gateState.position.x - gatePoseOrigin.position.x)*(gateState.position.x - gatePoseOrigin.position.x) +
                      (gateState.position.y - gatePoseOrigin.position.y)*(gateState.position.y - gatePoseOrigin.position.y) +
                      (gateState.position.z - gatePoseOrigin.position.z)*(gateState.position.z - gatePoseOrigin.position.z) );
}

void Localizer::publishDroneState(void)
{
    outState.position.x = inOdom.pose.pose.position.x + measGain * (gatePoseOrigin.position.x - gatePoseDrone.position.x - inOdom.pose.pose.position.x);
    outState.position.y = inOdom.pose.pose.position.y + measGain * (gatePoseOrigin.position.y - gatePoseDrone.position.y - inOdom.pose.pose.position.y);
    outState.position.z = inOdom.pose.pose.position.z + measGain * (gatePoseOrigin.position.z - gatePoseDrone.position.z - inOdom.pose.pose.position.z);


}