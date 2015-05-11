
robot_list(9)=load('robot_9.mat');
current_robot=robot_list(9);
robot=current_robot.robot;
[nothing total_robot]=size(robot);
test_voxel=[];test_dir=[];pre_dir=[];
 for i=48:total_robot
    if robot(i).fissure==0
current_box_vec=robot(i).ag_vision_box;
current_box_vec=current_box_vec(:);
%current_box_vec=[current_box_vec;robot(i).prev_x_dir;robot(i).prev_y_dir;robot(i).prev_z_dir;];
test_voxel=[test_voxel; current_box_vec'];
test_dir=[test_dir; robot(i).next_x_dir];
tmp_voxel=current_box_vec;
cur_alpha = regRF_predict(tmp_voxel',model_alpha);
cur_beta = regRF_predict(tmp_voxel',model_beta);
cur_mg = regRF_predict(tmp_voxel',model_mg);
[cur_x_dir, cur_y_dir, cur_z_dir]= sph2cart(cur_alpha,cur_beta,1);
pre_dir=[pre_dir; cur_x_dir];
    end
 end

corr(pre_dir,test_dir)

