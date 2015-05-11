function running_robots=robot_next_move(running_robots,ag_three_dim,box_size_define,zero_size)
[~,running_robots_num]=size(running_robots);
load('model_rotate.mat');

for i=1:running_robots_num
    disp(i);
    running_robots(i).previous.x_dir = running_robots(i).current.x_dir;
    running_robots(i).previous.y_dir = running_robots(i).current.y_dir;
    running_robots(i).previous.z_dir = running_robots(i).current.z_dir;
    ori_saved_box=extractbox(ag_three_dim, box_size_define, ...
        running_robots(i).current.x_loc,running_robots(i).current.y_loc,running_robots(i).current.z_loc,zero_size);
    tmp_voxel=ori_saved_box(:);
    %if sum(tmp_voxel) > 45
    running_robots(i).current.x_dir = model_x_dir.predict(tmp_voxel');
    running_robots(i).current.y_dir = model_y_dir.predict(tmp_voxel');
    running_robots(i).current.z_dir = model_z_dir.predict(tmp_voxel');
    running_robots(i).current.mg = model_mg.predict(tmp_voxel');
    [running_robots(i).current.x_dir, running_robots(i).current.y_dir, running_robots(i).current.x_dir...
        ]=decide_sign(running_robots(i).previous.x_dir,running_robots(i).previous.y_dir, ...
        running_robots(i).previous.z_dir, running_robots(i).current.x_dir, running_robots(i).current.y_dir, running_robots(i).current.z_dir);
    running_robots(i).next.x_loc=running_robots(i).current.x_loc+running_robots(i).current.x_dir*running_robots(i).current.mg;
    running_robots(i).next.y_loc=running_robots(i).current.y_loc+running_robots(i).current.y_dir*running_robots(i).current.mg;
    % running_robots(i).next.z_loc=running_robots(i).current.z_loc+(running_robots(i).current.z_dir-...
    % running_robots(i).previous.z_dir)*running_robots(i).current.mg;
    running_robots(i).next.z_loc=running_robots(i).current.z_loc+running_robots(i).current.z_dir*0.1;
    % line([running_robots(i).current.x_loc-zero_size, running_robots(i).next.x_loc-zero_size], [running_robots(i).current.y_loc-zero_size,...
    % running_robots(i).next.y_loc-zero_size], [running_robots(i).current.z_loc-zero_size, running_robots(i).next.z_loc-zero_size],'Color','b','LineWidth',5);
    line([running_robots(i).current.x_loc, running_robots(i).next.x_loc], [running_robots(i).current.y_loc,...
        running_robots(i).next.y_loc], [running_robots(i).current.z_loc, running_robots(i).next.z_loc],'Color','g','LineWidth',5);
    pause(0.1)
    running_robots(i).current.x_loc=running_robots(i).next.x_loc;
    running_robots(i).current.y_loc=running_robots(i).next.y_loc;
    running_robots(i).current.z_loc=running_robots(i).next.z_loc;
end

end
