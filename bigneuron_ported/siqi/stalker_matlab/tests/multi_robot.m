
clc
clear all
close all
addpath(genpath('C:\Users\donghao\Desktop\Sydney\new_construction'));
tic
robot_list(9)=load('robot_9.mat');
current_robot=robot_list(9);
robot=current_robot.robot;
load('OP_9_ag_three_dim.mat');
load('OP_9_centroid.mat');
zero_size=20;
box_size_define=13;
%uiopen('C:\Users\donghao\Desktop\Sydney\new_construction\OP_9_gt.fig',1)
uiopen('C:\Users\donghao\Desktop\Sydney\new_construction\OP_9_voxel.fig',1)
toc
hold on
%running_robot begin here
[running_robot_number,useless]=size(centroid);
for i=1:running_robot_number
running_robot(i).current.x_loc=centroid(i,1);
running_robot(i).current.y_loc=centroid(i,2);
running_robot(i).current.z_loc=centroid(i,3);
end
tic
running_robot=initialize_robot(running_robot,ag_three_dim,box_size_define,zero_size);
toc
tic
for i=1:20
running_robot=robot_next_move(running_robot,ag_three_dim,box_size_define,zero_size);
end
toc
