#include <cstdlib>
#include <iostream>
#include "../header/Aa_faceDetection.h"

Aa_faceDetection::Aa_faceDetection(ros::NodeHandle nh, ros::Publisher pubPlat){
  detectedLeft = false;
  detectedFaceConsec = 0;
  this->nh = nh;
  this->pubPlat = pubPlat;

}

void Aa_faceDetection::subImgDetectCallbackRight(const opencv_apps::FaceArrayStamped &msgFacesRight){

  look_caresses_pkg::platform_control msgVel;

  if (msgFacesRight.faces.empty()) { //no detection with right camera
    if(detectedLeft) {
      //turn slowly clockwise to catch face also with right camera
      msgVel.body_vel.angular.z = 0.1;
      detectedFaceConsec = 0; //reset counter time for detect face
    }
    else {
      //turn a little faster clockwise because neither left camera has detected face
      msgVel.body_vel.angular.z = 0.2;
      detectedFaceConsec = 0;
    }
  }
  else {
    if(detectedLeft) {
      //face detected with both cameras, keep position
      msgVel.body_vel.angular.z = 0;
      detectedFaceConsec++; // to check the time the face is in range
    }
    else {
      //turn slowly counterclockwise to catch face also with left camera
      msgVel.body_vel.angular.z = -0.1;
      detectedFaceConsec = 0;
    }
  }

  pubPlat.publish(msgVel);
}

void Aa_faceDetection::subImgDetectCallbackLeft(const opencv_apps::FaceArrayStamped &msgFacesLeft){

  if (msgFacesLeft.faces.empty()) {
    detectedLeft = false;
  }
  else {
    detectedLeft = true;
  }
}

int Aa_faceDetection::main()
{

    detectedLeft = false;
    detectedFaceConsec = 0;

    ROS_INFO("[Aa] Started");
    subTopics();

    ros::Rate loop_rate(10); //10 hz
    while (detectedFaceConsec <= 30){ //detect face and keep it in focus for 30*10hz = 3 seconds
      ros::spinOnce();
      loop_rate.sleep();
    }
    look_caresses_pkg::platform_control plat_msgs_saw;
    plat_msgs_saw.sound_index_P2 = rand()%20; //pirate sound
    pubPlat.publish(plat_msgs_saw);
    ROS_INFO("[Aa] I saw you consecutevely for 3 seconds");

    unsubTopics();
    ROS_INFO("[Aa] Finished");
    return 0;
}

void Aa_faceDetection::subTopics(){

  subImageDetectRight = nh.subscribe("/right/face_detection/faces", 1000, &Aa_faceDetection::subImgDetectCallbackRight, this);
  subImageDetectLeft = nh.subscribe("/left/face_detection/faces", 1000, &Aa_faceDetection::subImgDetectCallbackLeft, this);

}

void Aa_faceDetection::unsubTopics(){
  subImageDetectRight.shutdown();
  subImageDetectLeft.shutdown();
}

