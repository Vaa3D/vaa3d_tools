
clc
clear all
%close all
addpath(genpath('C:\Users\donghao\Desktop\Sydney\new_construction'));
robot_list(1)=load('robot_1.mat');
robot_list(2)=load('robot_2.mat');
robot_list(3)=load('robot_3.mat');
robot_list(4)=load('robot_4.mat');
robot_list(5)=load('robot_5.mat');
robot_list(6)=load('robot_6.mat');
robot_list(7)=load('robot_7.mat');
robot_list(8)=load('robot_8.mat');
robot_list(9)=load('robot_9.mat');
shift_robot_list(1)=load('shift_robot_1.mat');
shift_robot_list(2)=load('shift_robot_2.mat');
shift_robot_list(3)=load('shift_robot_3.mat');
shift_robot_list(4)=load('shift_robot_4.mat');
shift_robot_list(5)=load('shift_robot_5.mat');
shift_robot_list(6)=load('shift_robot_6.mat');
shift_robot_list(7)=load('shift_robot_7.mat');
shift_robot_list(8)=load('shift_robot_8.mat');
shift_robot_list(9)=load('shift_robot_9.mat');

%train all robots
tic
x_trn=[];

alpha_gt=[];
beta_gt=[];
mg_gt=[];
for j_1=1:9
    easy_case=[1 2 3 4 5 6 7 8 9];
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
alpha_gt=[alpha_gt; robot(i).next_alpha];
beta_gt=[beta_gt; robot(i).next_beta];

mg_gt=[mg_gt; robot(i).next_mag];
    end
    
end


current_shift_robot=shift_robot_list(j);
current_shift_robot=current_shift_robot.shift_robot;
%begin to load data 
[useless, all_shift_robot]=size(current_shift_robot);
for i=1:all_shift_robot
i
current_box_vec=current_shift_robot(i).ag_vision_box;
current_box_vec=current_box_vec(:);
x_trn=[x_trn; current_box_vec'];
alpha_gt=[alpha_gt; current_shift_robot(i).next_alpha];
beta_gt=[beta_gt; current_shift_robot(i).next_beta];
mg_gt=[mg_gt; current_shift_robot(i).next_mg];
end

end
toc




tic
model_alpha = regRF_train(x_trn,alpha_gt,200,70);
toc
tic
model_beta = regRF_train(x_trn,beta_gt,200,70);
toc

tic
model_mg = regRF_train(x_trn,mg_gt,200,70);
toc
%  x_trn=[];
%  y_trn=[];
robot_list(9)=load('robot_9.mat');
current_robot=robot_list(9);
robot=current_robot.robot;
load('OP_9_ag_three_dim.mat');
%figure
hold on
zero_size=20;
box_size_define=13;
current.x_loc=76.663;current.y_loc=324;current.z_loc=9.749;
%current.x_loc=99.13; current.y_loc=302.3; current.z_loc=10.794;
%current.x_loc=40.281; current.y_loc=323.11; current.z_loc=26.642;
ori_saved_box=extractbox(ag_three_dim, box_size_define, current.x_loc,current.y_loc,current.z_loc,zero_size);
tmp_voxel=ori_saved_box(:);
cur_alpha = regRF_predict(tmp_voxel',model_alpha);
cur_beta = regRF_predict(tmp_voxel',model_beta);
[cur_x_dir, cur_y_dir, cur_z_dir]= sph2cart(cur_alpha,cur_beta,1);
cur_y_dir=-cur_y_dir;
for step=1:40
ori_saved_box=extractbox(ag_three_dim, box_size_define, current.x_loc,current.y_loc,current.z_loc,zero_size);
tmp_voxel=ori_saved_box(:);

pre_x_dir=cur_x_dir;
pre_y_dir=cur_y_dir;
pre_z_dir=cur_z_dir;
pre_alpha = cur_alpha;
pre_beta = cur_beta;
cur_alpha = regRF_predict(tmp_voxel',model_alpha);
cur_beta = regRF_predict(tmp_voxel',model_beta);

cur_mg = regRF_predict(tmp_voxel',model_mg);
[cur_x_dir, cur_y_dir, cur_z_dir]= sph2cart(cur_alpha,cur_beta,1);
%[pre_x_dir, pre_y_dir, pre_z_dir]= sph2cart(cur_alpha,cur_beta,1);
[cur_x_dir, cur_y_dir, cur_z_dir]=decide_sign(pre_x_dir, pre_y_dir, pre_z_dir, cur_x_dir, cur_y_dir, cur_z_dir);
%[cur_alpha,cur_beta]=decide_angle_sign(pre_alpha,pre_beta,cur_alpha,cur_beta);

% cur_mg=2;
next.x_loc=current.x_loc+cur_x_dir*cur_mg;
next.y_loc=current.y_loc+cur_y_dir*cur_mg;
next.z_loc=current.z_loc+cur_z_dir*cur_mg*0.1;
line([current.x_loc, next.x_loc], [current.y_loc,next.y_loc], [current.z_loc, next.z_loc],'Color','b','LineWidth',5);
pause(0.5)
current.x_loc=next.x_loc;
current.y_loc=next.y_loc;
current.z_loc=next.z_loc;
end
