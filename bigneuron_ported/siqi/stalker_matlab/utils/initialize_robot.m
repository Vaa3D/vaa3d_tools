function running_robot=initialize_robot(running_robot,ag_three_dim,box_size_define,zero_size)
[~,running_robot_num]=size(running_robot);
load('model_rotate.mat');
for i=1:running_robot_num
ori_saved_box=extractbox(ag_three_dim, box_size_define, ...
running_robot(i).current.x_loc,running_robot(i).current.y_loc,running_robot(i).current.z_loc,zero_size);
tmp_voxel=ori_saved_box(:);
running_robot(i).current.x_dir = model_x_dir.predict(tmp_voxel');
running_robot(i).current.y_dir = model_y_dir.predict(tmp_voxel');
running_robot(i).current.z_dir = model_z_dir.predict(tmp_voxel');
[running_robot(i).current.x_dir,running_robot(i).current.y_dir,running_robot(i).current.z_dir...
]=rand_dir(running_robot(i).current.x_dir,running_robot(i).current.y_dir,running_robot(i).current.z_dir);
running_robot(i).previous.x_dir = running_robot(i).current.x_dir;
running_robot(i).previous.y_dir = running_robot(i).current.y_dir;
running_robot(i).previous.z_dir = running_robot(i).current.z_dir;
end

end