#EECS 373 PS4

For this assignment, start up a simulation from learning_ros with the commands (in separate terminals):

roslaunch gazebo_ros empty_world.launch
roslaunch mobot_urdf mobot_w_lidar.launch
rosrun mobot_pub_des_state open_loop_controller
rosrun mobot_pub_des_state mobot_pub_des_state
rosrun mobot_pub_des_state pub_des_state_path_client

The "pub_des_state" node is the heart of this simulation.  It receives desired paths from a client (pub_des_state_path_client, in this instance, which requests motion in a 5mx5m square).  Pub_des_state processes such requests by streaming out sequences of desired trajectories.  At present, these are commanded to the robot open-loop via the node "open_loop_controller".

The pub_des_state node (described in more detail in our text, Ch9) uses the library "traj_builder", which creates smooth trajectory plans from course via points (vertices of polylines).  You should finish the function: build_braking_traj() in traj_builder.

Additionally, create a new version of your lidar-alarm node which evaluates if a collision is impending, and if so, invokes an "e-stop" (via the e-stop service of pub_des_state).
