clear all
clc
close all
warning off;
addpath(genpath('C:\Users\donghao\Desktop\Sydney\new_construction'));
cd Image_Stacks
Number_file=length(dir('OP_1'))-3;
cd OP_1
raw_image_prep
cd ..
cd ..
save OP_1_three_dim three_dim
save OP_1_salt_pepper_three_dim salt_pepper_three_dim
save OP_1_ag_three_dim ag_three_dim
clc
clear all
load('OP_1_salt_pepper_three_dim.mat');
 zero_size=20;
 % salt_pepper_three_dim=padarray(salt_pepper_three_dim,[zero_size,zero_size,zero_size]);
load('OP_1_ag_three_dim.mat');
 % ag_three_dim=padarray(ag_three_dim,[zero_size,zero_size,zero_size]);
load('OP_1_three_dim.mat');
 % three_dim=padarray(three_dim,[zero_size,zero_size,zero_size]);
cd ground_truth
importdata('C:\Users\donghao\Desktop\Sydney\new_construction\ground_truth\Copy_of_OP_1.swc');
savetree
cd ..

%  %% Ready to train 
[nothing total_robot]=size(robot);
save robot_1 robot



clear all
clc
close all
warning off;
addpath(genpath('C:\Users\donghao\Desktop\Sydney\new_construction'));
cd Image_Stacks
Number_file=length(dir('OP_2'))-3;
cd OP_2
raw_image_prep
cd ..
cd ..
save OP_2_three_dim three_dim
save OP_2_salt_pepper_three_dim salt_pepper_three_dim
save OP_2_ag_three_dim ag_three_dim
clc
clear all
load('OP_2_salt_pepper_three_dim.mat');
 zero_size=20;
 % salt_pepper_three_dim=padarray(salt_pepper_three_dim,[zero_size,zero_size,zero_size]);
load('OP_2_ag_three_dim.mat');
 % ag_three_dim=padarray(ag_three_dim,[zero_size,zero_size,zero_size]);
load('OP_2_three_dim.mat');
 % three_dim=padarray(three_dim,[zero_size,zero_size,zero_size]);
cd ground_truth
importdata('C:\Users\donghao\Desktop\Sydney\new_construction\ground_truth\Copy_of_OP_2.swc');
savetree
cd ..

%  %% Ready to train 
[nothing total_robot]=size(robot);
save robot_2 robot



clear all
clc
close all
warning off;
addpath(genpath('C:\Users\donghao\Desktop\Sydney\new_construction'));
cd Image_Stacks
Number_file=length(dir('OP_3'))-3;
cd OP_3
raw_image_prep
cd ..
cd ..
save OP_3_three_dim three_dim
save OP_3_salt_pepper_three_dim salt_pepper_three_dim
save OP_3_ag_three_dim ag_three_dim
clc
clear all
load('OP_3_salt_pepper_three_dim.mat');
 zero_size=20;
 % salt_pepper_three_dim=padarray(salt_pepper_three_dim,[zero_size,zero_size,zero_size]);
load('OP_3_ag_three_dim.mat');
 % ag_three_dim=padarray(ag_three_dim,[zero_size,zero_size,zero_size]);
load('OP_3_three_dim.mat');
 % three_dim=padarray(three_dim,[zero_size,zero_size,zero_size]);
cd ground_truth
importdata('C:\Users\donghao\Desktop\Sydney\new_construction\ground_truth\Copy_of_OP_3.swc');
savetree
cd ..

%  %% Ready to train 
[nothing total_robot]=size(robot);
save robot_3 robot



clear all
clc
close all
warning off;
addpath(genpath('C:\Users\donghao\Desktop\Sydney\new_construction'));
cd Image_Stacks
Number_file=length(dir('OP_4'))-3;
cd OP_4
raw_image_prep
cd ..
cd ..
save OP_4_three_dim three_dim
save OP_4_salt_pepper_three_dim salt_pepper_three_dim
save OP_4_ag_three_dim ag_three_dim
clc
clear all
load('OP_4_salt_pepper_three_dim.mat');
 zero_size=20;
 % salt_pepper_three_dim=padarray(salt_pepper_three_dim,[zero_size,zero_size,zero_size]);
load('OP_4_ag_three_dim.mat');
 % ag_three_dim=padarray(ag_three_dim,[zero_size,zero_size,zero_size]);
load('OP_4_three_dim.mat');
 % three_dim=padarray(three_dim,[zero_size,zero_size,zero_size]);
cd ground_truth
importdata('C:\Users\donghao\Desktop\Sydney\new_construction\ground_truth\Copy_of_OP_4.swc');
savetree
cd ..

%  %% Ready to train 
[nothing total_robot]=size(robot);
save robot_4 robot


clear all
clc
close all
warning off;
addpath(genpath('C:\Users\donghao\Desktop\Sydney\new_construction'));
cd Image_Stacks
Number_file=length(dir('OP_5'))-3;
cd OP_5
raw_image_prep
cd ..
cd ..
save OP_5_three_dim three_dim
save OP_5_salt_pepper_three_dim salt_pepper_three_dim
save OP_5_ag_three_dim ag_three_dim
clc
clear all
load('OP_5_salt_pepper_three_dim.mat');
 zero_size=20;
 % salt_pepper_three_dim=padarray(salt_pepper_three_dim,[zero_size,zero_size,zero_size]);
load('OP_5_ag_three_dim.mat');
 % ag_three_dim=padarray(ag_three_dim,[zero_size,zero_size,zero_size]);
load('OP_5_three_dim.mat');
 % three_dim=padarray(three_dim,[zero_size,zero_size,zero_size]);
cd ground_truth
importdata('C:\Users\donghao\Desktop\Sydney\new_construction\ground_truth\Copy_of_OP_5.swc');
savetree
cd ..

%  %% Ready to train 
[nothing total_robot]=size(robot);
save robot_5 robot



clear all
clc
close all
warning off;
addpath(genpath('C:\Users\donghao\Desktop\Sydney\new_construction'));
cd Image_Stacks
Number_file=length(dir('OP_6'))-3;
cd OP_6
raw_image_prep
cd ..
cd ..
save OP_6_three_dim three_dim
save OP_6_salt_pepper_three_dim salt_pepper_three_dim
save OP_6_ag_three_dim ag_three_dim
clc
clear all
load('OP_6_salt_pepper_three_dim.mat');
 zero_size=20;
 % salt_pepper_three_dim=padarray(salt_pepper_three_dim,[zero_size,zero_size,zero_size]);
load('OP_6_ag_three_dim.mat');
 % ag_three_dim=padarray(ag_three_dim,[zero_size,zero_size,zero_size]);
load('OP_6_three_dim.mat');
 % three_dim=padarray(three_dim,[zero_size,zero_size,zero_size]);
cd ground_truth
importdata('C:\Users\donghao\Desktop\Sydney\new_construction\ground_truth\Copy_of_OP_6.swc');
savetree
cd ..

%  %% Ready to train 
[nothing total_robot]=size(robot);
save robot_6 robot


clear all
clc
close all
warning off;
addpath(genpath('C:\Users\donghao\Desktop\Sydney\new_construction'));
cd Image_Stacks
Number_file=length(dir('OP_7'))-3;
cd OP_7
raw_image_prep
cd ..
cd ..
save OP_7_three_dim three_dim
save OP_7_salt_pepper_three_dim salt_pepper_three_dim
save OP_7_ag_three_dim ag_three_dim
clc
clear all
load('OP_7_salt_pepper_three_dim.mat');
 zero_size=20;
 % salt_pepper_three_dim=padarray(salt_pepper_three_dim,[zero_size,zero_size,zero_size]);
load('OP_7_ag_three_dim.mat');
 % ag_three_dim=padarray(ag_three_dim,[zero_size,zero_size,zero_size]);
load('OP_7_three_dim.mat');
 % three_dim=padarray(three_dim,[zero_size,zero_size,zero_size]);
cd ground_truth
importdata('C:\Users\donghao\Desktop\Sydney\new_construction\ground_truth\Copy_of_OP_7.swc');
savetree
cd ..

%  %% Ready to train 
[nothing total_robot]=size(robot);
save robot_7 robot


clear all
clc
close all
warning off;
addpath(genpath('C:\Users\donghao\Desktop\Sydney\new_construction'));
cd Image_Stacks
Number_file=length(dir('OP_8'))-3;
cd OP_8
raw_image_prep
cd ..
cd ..
save OP_8_three_dim three_dim
save OP_8_salt_pepper_three_dim salt_pepper_three_dim
save OP_8_ag_three_dim ag_three_dim
clc
clear all
load('OP_8_salt_pepper_three_dim.mat');
 zero_size=20;
 % salt_pepper_three_dim=padarray(salt_pepper_three_dim,[zero_size,zero_size,zero_size]);
load('OP_8_ag_three_dim.mat');
 % ag_three_dim=padarray(ag_three_dim,[zero_size,zero_size,zero_size]);
load('OP_8_three_dim.mat');
 % three_dim=padarray(three_dim,[zero_size,zero_size,zero_size]);
cd ground_truth
importdata('C:\Users\donghao\Desktop\Sydney\new_construction\ground_truth\Copy_of_OP_8.swc');
savetree
cd ..

%  %% Ready to train 
[nothing total_robot]=size(robot);
save robot_8 robot

clear all
clc
close all
warning off;
addpath(genpath('C:\Users\donghao\Desktop\Sydney\new_construction'));
cd Image_Stacks
Number_file=length(dir('OP_9'))-3;
cd OP_9
raw_image_prep
cd ..
cd ..
save OP_9_three_dim three_dim
save OP_9_salt_pepper_three_dim salt_pepper_three_dim
save OP_9_ag_three_dim ag_three_dim
clc
clear all
load('OP_9_salt_pepper_three_dim.mat');
 zero_size=20;
 % salt_pepper_three_dim=padarray(salt_pepper_three_dim,[zero_size,zero_size,zero_size]);
load('OP_9_ag_three_dim.mat');
 % ag_three_dim=padarray(ag_three_dim,[zero_size,zero_size,zero_size]);
load('OP_9_three_dim.mat');
 % three_dim=padarray(three_dim,[zero_size,zero_size,zero_size]);
cd ground_truth
importdata('C:\Users\donghao\Desktop\Sydney\new_construction\ground_truth\Copy_of_OP_9.swc');
savetree
cd ..

%  %% Ready to train 
[nothing total_robot]=size(robot);
save robot_9 robot