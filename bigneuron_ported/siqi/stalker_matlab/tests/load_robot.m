tic
clc
clear all
close all
addpath(genpath('C:\Users\donghao\Desktop\Sydney\new_construction'));
robot_list(1)=load('robot_1.mat');
robot_list(2)=load('robot_2.mat');
robot_list(3)=load('robot_3.mat');
robot_list(4)=load('robot_4.mat');
robot_list(5)=load('robot_5.mat');
robot_list(6)=load('robot_6.mat');
robot_list(7)=load('robot_7.mat');
robot_list(8)=load('robot_8.mat');

toc

%train all robots
tic
x_trn=[];
y_trn=[];
for j_1=1:7
    easy_case=[1 3 4 5 6 7 8];
    j=easy_case(j_1);
current_robot=robot_list(j);
robot=current_robot.robot;
[nothing total_robot]=size(robot);
for i=1:total_robot
    i
    if robot(i).fissure==0
current_box_vec=robot(i).ag_vision_box;
current_box_vec=current_box_vec(:);
%current_box_vec=[current_box_vec;robot(i).prev_x_dir;robot(i).prev_y_dir;robot(i).prev_z_dir;];
x_trn=[x_trn; current_box_vec'];
y_trn=[y_trn; robot(i).next_x_dir];
current_box_vec=robot(i).ag_vision_box_x_plus_one;
current_box_vec=current_box_vec(:);
x_trn=[x_trn; current_box_vec'];
y_trn=[y_trn; robot(i).next_x_dir];
current_box_vec=robot(i).ag_vision_box_x_minus_one;
current_box_vec=current_box_vec(:);
x_trn=[x_trn; current_box_vec'];
y_trn=[y_trn; robot(i).next_x_dir];
current_box_vec=robot(i).ag_vision_box_y_plus_one;
current_box_vec=current_box_vec(:);
x_trn=[x_trn; current_box_vec'];
y_trn=[y_trn; robot(i).next_x_dir];
current_box_vec=robot(i).ag_vision_box_y_minus_one;
current_box_vec=current_box_vec(:);
x_trn=[x_trn; current_box_vec'];
y_trn=[y_trn; robot(i).next_x_dir];
current_box_vec=robot(i).ag_vision_box_z_plus_one;
current_box_vec=current_box_vec(:);
x_trn=[x_trn; current_box_vec'];
y_trn=[y_trn; robot(i).next_x_dir];
current_box_vec=robot(i).ag_vision_box_z_minus_one;
current_box_vec=current_box_vec(:);
x_trn=[x_trn; current_box_vec'];
y_trn=[y_trn; robot(i).next_x_dir];
    end
end
end
toc 


% clear robot 
% clear robot_list
% 
% % extra_options.do_trace = 1;
% % extra_options.keep_inbag = 1;
% %  9261 tree 200 mtry 400
% %  9261 tree 200 mtry 800 Elapsed time is 509.764245 seconds.
% %  9261 tree 200 mtry 1600 Elapsed time is 1002.270420 seconds.
% %  9261 tree 200 mtry 2000 Elapsed time is 1258.708595 seconds.
% %  9261 tree 400 mtry 2000 Elapsed time is 2486.109733 seconds. corr 0.76
tic
model = regRF_train(x_trn,y_trn,50,70);
toc
tic
test_voxel=[];
test_dir=[];
robot_list(9)=load('robot_9.mat');
current_robot=robot_list(9);
robot=current_robot.robot;
[nothing total_robot]=size(robot);
 for i=30:total_robot
     %i
    if robot(i).fissure==0
current_box_vec=robot(i).ag_vision_box;
current_box_vec=current_box_vec(:);
%current_box_vec=[current_box_vec;robot(i).prev_x_dir;robot(i).prev_y_dir;robot(i).prev_z_dir;];
test_voxel=[test_voxel; current_box_vec'];
test_dir=[test_dir; robot(i).next_x_dir];
    end
 end
pre_dir = regRF_predict(test_voxel,model);
corr(pre_dir,test_dir)
toc