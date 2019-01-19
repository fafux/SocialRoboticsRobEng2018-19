#include "look_caresses_pkg/platform_control.h"
#include <iostream>
#include <string>
#include <numeric>
#include "../header/B_approach.h"

B_approach::B_approach(int argc, char **argv){
  ros::init(argc, argv, "B_approach");
  ros::NodeHandle nh;
  sonarMsgs = boost::circular_buffer<float>(averageNum);
  counter = 0;

  pubPlat = nh.advertise<look_caresses_pkg::platform_control>("/miro/rob01/platform/control", 1000);
  pubVel = nh.advertise<look_caresses_pkg::platform_control>("/miro/rob01/platform/control",100);
  subRange = nh.subscribe("/miro/rob01/sensors/sonar_range", 1000, &B_approach::sonarCallback, this);
}

void B_approach::sonarCallback(const sensor_msgs::Range &sensor_range)
{
    look_caresses_pkg::platform_control msg;
    sonarMsgs.push_front(sensor_range.range);

    if (sonarMsgs.full()){ //wait to have "averageNum" values of range

      // average between more than 1 value to avoid the errors of the sonar (sometimes returns zeros)
      float sum = std::accumulate(sonarMsgs.begin(), sonarMsgs.end(), 0.0); //0 init the sum to 0
      float average = sum/averageNum;
      ROS_INFO("Sonar Range average: %f", average);

      if (average > 0.15) { // in meters
        msg.body_vel.linear.x = 30; // good choiche for the linear velocity
        msg.body_vel.angular.z = 0.05; // because miro turns unwanted
      }
      else { // miro must not move
        msg.body_vel.linear.x = 0;
        msg.body_vel.angular.z = 0;
        counter ++;
      }
    } else { // miro must not move
      msg.body_vel.linear.x = 0;
      msg.body_vel.angular.z = 0;
    }
    pubVel.publish(msg);
}

int B_approach::main()
{
    // Publish the kinematic of the head to position the head up for detecting sonar range
    ros::Rate loop_rate(1);
    look_caresses_pkg::platform_control plat_msgs_headup;
    float body_config_headup[4] = {0.0, 0.4, 0, -0.1};
    float body_config_speed_headup[4] = {0.0, -1.0, -1.0, -1.0};
    for (int i =0; i<4; i++){
      plat_msgs_headup.body_config[i] = body_config_headup[i];
      plat_msgs_headup.body_config_speed[i] = body_config_speed_headup[i];
    }
    loop_rate.sleep();
    pubPlat.publish(plat_msgs_headup);

    ros::Rate loop_rate2(2);
    while (ros::ok() && counter<6){
      ros::spinOnce();
      loop_rate2.sleep();
    }

    //ros::spin();

    return 0;
}
