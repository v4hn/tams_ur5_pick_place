#include <ros/ros.h>
#include <manipulation_msgs/GraspPlanning.h>

#include <moveit/move_group_interface/move_group.h>

void jointValuesToJointTrajectory(std::map<std::string, double> target_values, sensor_msgs::JointState &grasp_pose)
{
  grasp_pose.name.reserve(target_values.size());
  grasp_pose.position.reserve(target_values.size());

  for(std::map<std::string, double>::iterator it = target_values.begin(); it != target_values.end(); ++it){
    grasp_pose.name.push_back(it->first);
    grasp_pose.position.push_back(it->second);
  }
}

bool serviceCB(manipulation_msgs::GraspPlanning::Request  &req,
         manipulation_msgs::GraspPlanning::Response &res)
{
  moveit::planning_interface::MoveGroup arm(req.arm_name);
  moveit::planning_interface::MoveGroup gripper(arm.getRobotModel()->getEndEffectors()[0]->getName());

  manipulation_msgs::Grasp grasp;
  grasp.id = "grasp";

  jointValuesToJointTrajectory(gripper.getNamedTargetValues("open"), grasp.pre_grasp_posture);
  jointValuesToJointTrajectory(gripper.getNamedTargetValues("closed"), grasp.grasp_posture);

  geometry_msgs::PoseStamped pose;
  pose.header.frame_id = req.collision_object_name;
  pose.pose.orientation.x = 0.5;
  pose.pose.orientation.y = 0.5;
  pose.pose.orientation.z = -0.5;
  pose.pose.orientation.w = 0.5;
  pose.pose.position.z = 0.1;
  grasp.grasp_pose = pose;

  grasp.approach.min_distance = 0.02;
  grasp.approach.desired_distance = 0.1;
  grasp.approach.direction.header.frame_id = "tool0";
  grasp.approach.direction.vector.x = 1.0;

  grasp.retreat.min_distance = 0.02;
  grasp.retreat.desired_distance = 0.1;
  grasp.retreat.direction.header.frame_id = arm.getPlanningFrame();
  grasp.retreat.direction.vector.z = 1.0;

  res.grasps.push_back(grasp);
  return true;
}


int main(int argc, char **argv)
{
  ros::init(argc, argv, "grasp_planning_server");
  ros::NodeHandle n;

  ros::ServiceServer service = n.advertiseService("database_grasp_planning", serviceCB);
  ros::spin();

  return 0;
}