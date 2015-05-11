% clear all
% close all
% clc
%add raw voxel and raw voxel with noises

showbox(ag_three_dim,0.1)
index_ori=ans.data(:,1);
type1=ans.data(:,2);
x_location=ans.data(:,3);
y_location=ans.data(:,4);
z_location=ans.data(:,5);
r=ans.data(:,6);
parind=ans.data(:,7);

for i=1:numel(parind)
    robot_ground_truth(i).type=type1(i);
    robot_ground_truth(i).radius=r(i);
    robot_ground_truth(i).x_loc=x_location(i);
    robot_ground_truth(i).y_loc=y_location(i);
    robot_ground_truth(i).z_loc=z_location(i);
    robot_ground_truth(i).p_index_ori=parind(i);
    robot_ground_truth(i).index_ori=index_ori(i);
end

t = tree(robot_ground_truth(1));
[t n(1)] = t.addnode(1, robot_ground_truth(2));
for i=2:1:numel(parind)-1
    par_n=parind(i+1);
    par_n=par_n-1;
    [t n(i)] = t.addnode(n(par_n), robot_ground_truth(i+1));
end

figure
hold on
for i=1:(numel(parind)-1)
    test=t.get(n(i));
    plot3(test.x_loc, test.y_loc, test.z_loc, 'r+');
    %pause(0.1)
end

df_order = tree(t, 'clear'); % Generate an empty synchronized tree
iterator = t.depthfirstiterator; % Doesn't matter whether you call this on |t| or |df_order|
% index = 1;
% for i = iterator
%     df_order = df_order.set(i, index);
%     index = index + 1;
% end
%disp(df_order.tostring)
% disp(t.tostring)
iterator=iterator-1;

% for i=50
% node_ind_next=t.getchildren(n(i))
% end
%for i=2:numel(parind)-1

%initialize the root parameter
box_size_define=13;

% figure
% showbox(ag_three_dim,0.3)




current=t.get(1);
ori_saved_box=extractbox(three_dim, box_size_define, current.x_loc,current.y_loc,current.z_loc,zero_size);
robot(1).ori_vision_box=ori_saved_box;
ori_saved_box=extractbox(salt_pepper_three_dim, box_size_define, current.x_loc,current.y_loc,current.z_loc,zero_size);
robot(1).sp_vision_box=ori_saved_box;
ori_saved_box=extractbox(ag_three_dim, box_size_define, current.x_loc,current.y_loc,current.z_loc,zero_size);
robot(1).ag_vision_box=ori_saved_box;
% ori_saved_box=extractbox(ag_three_dim, box_size_define, (current.x_loc-1),current.y_loc,current.z_loc,zero_size);
% robot(1).ag_vision_box_x_minus_one=ori_saved_box;
% ori_saved_box=extractbox(ag_three_dim, box_size_define, (current.x_loc+1),current.y_loc,current.z_loc,zero_size);
% robot(1).ag_vision_box_x_plus_one=ori_saved_box;
% ori_saved_box=extractbox(ag_three_dim, box_size_define, current.x_loc,(current.y_loc-1),current.z_loc,zero_size);
% robot(1).ag_vision_box_y_minus_one=ori_saved_box;
% ori_saved_box=extractbox(ag_three_dim, box_size_define, current.x_loc,(current.y_loc+1),current.z_loc,zero_size);
% robot(1).ag_vision_box_y_plus_one=ori_saved_box;
% ori_saved_box=extractbox(ag_three_dim, box_size_define, current.x_loc,current.y_loc,(current.z_loc-1),zero_size);
% robot(1).ag_vision_box_z_minus_one=ori_saved_box;
% ori_saved_box=extractbox(ag_three_dim, box_size_define, current.x_loc,current.y_loc,(current.z_loc+1),zero_size);
% robot(1).ag_vision_box_z_plus_one=ori_saved_box;
robot(1).fissure=0;


current=t.get(n(1));
ori_saved_box=extractbox(three_dim, box_size_define, current.x_loc,current.y_loc,current.z_loc,zero_size);
robot(2).ori_vision_box=ori_saved_box;
ori_saved_box=extractbox(salt_pepper_three_dim, box_size_define, current.x_loc,current.y_loc,current.z_loc,zero_size);
robot(2).sp_vision_box=ori_saved_box;
ori_saved_box=extractbox(ag_three_dim, box_size_define, current.x_loc,current.y_loc,current.z_loc,zero_size);
robot(2).ag_vision_box=ori_saved_box;
%  ori_saved_box=extractbox(ag_three_dim, box_size_define, (current.x_loc-1),current.y_loc,current.z_loc,zero_size);
%  robot(2).ag_vision_box_x_minus_one=ori_saved_box;
%  ori_saved_box=extractbox(ag_three_dim, box_size_define, (current.x_loc+1),current.y_loc,current.z_loc,zero_size);
%  robot(2).ag_vision_box_x_plus_one=ori_saved_box;
%  ori_saved_box=extractbox(ag_three_dim, box_size_define, current.x_loc,(current.y_loc-1),current.z_loc,zero_size);
%  robot(2).ag_vision_box_y_minus_one=ori_saved_box;
%  ori_saved_box=extractbox(ag_three_dim, box_size_define, current.x_loc,(current.y_loc+1),current.z_loc,zero_size);
%  robot(2).ag_vision_box_y_plus_one=ori_saved_box;
%  ori_saved_box=extractbox(ag_three_dim, box_size_define, current.x_loc,current.y_loc,(current.z_loc-1),zero_size);
%  robot(2).ag_vision_box_z_minus_one=ori_saved_box;
%  ori_saved_box=extractbox(ag_three_dim, box_size_define, current.x_loc,current.y_loc,(current.z_loc+1),zero_size);
%  robot(2).ag_vision_box_z_plus_one=ori_saved_box;
robot(2).fissure=0;

%initilize the unit vector
current=t.get(1);
next=t.get(n(1));
next_x=next.x_loc-current.x_loc;
next_y=next.y_loc-current.y_loc;
next_z=next.z_loc-current.z_loc;
next_magnitude=sqrt(next_x*next_x+next_y*next_y+next_z*next_z);
next_x_direction=next_x/next_magnitude;
next_y_direction=next_y/next_magnitude;
next_z_direction=next_z/next_magnitude;
robot(1).next_x_dir=next_x_direction;
robot(1).next_y_dir=next_y_direction;
robot(1).next_z_dir=next_z_direction;
robot(1).next_mag=next_magnitude;
robot(1).radius=current.radius;

robot(1).prev_x_dir=next_x_direction;
robot(1).prev_y_dir=next_y_direction;
robot(1).prev_z_dir=next_z_direction;
robot(1).prev_mag=next_magnitude;





%initialize the children parameter
for   i=2:numel(parind)-1
    
    node_ind=iterator(i);
    %node_ind_next=iterator(i+1);
    current=t.get(n(node_ind));
    robot(i).radius=current.radius;
    ori_saved_box=extractbox(three_dim, box_size_define, current.x_loc,current.y_loc,current.z_loc,zero_size);
    %first parameter done
    robot(i).ori_vision_box=ori_saved_box;
    
    ori_saved_box=extractbox(salt_pepper_three_dim, box_size_define, current.x_loc,current.y_loc,current.z_loc,zero_size);
    %second parameter done
    robot(i).sp_vision_box=ori_saved_box;
    
    ori_saved_box=extractbox(ag_three_dim, box_size_define, current.x_loc,current.y_loc,current.z_loc,zero_size);
    %third parameter done
    robot(i).ag_vision_box=ori_saved_box;
    %Start to get more box from the picture
    % ori_saved_box=extractbox(ag_three_dim, box_size_define, (current.x_loc-1),current.y_loc,current.z_loc,zero_size);
    % robot(i).ag_vision_box_x_minus_one=ori_saved_box;
    % ori_saved_box=extractbox(ag_three_dim, box_size_define, (current.x_loc+1),current.y_loc,current.z_loc,zero_size);
    % robot(i).ag_vision_box_x_plus_one=ori_saved_box;
    % ori_saved_box=extractbox(ag_three_dim, box_size_define, current.x_loc,(current.y_loc-1),current.z_loc,zero_size);
    % robot(i).ag_vision_box_y_minus_one=ori_saved_box;
    % ori_saved_box=extractbox(ag_three_dim, box_size_define, current.x_loc,(current.y_loc+1),current.z_loc,zero_size);
    % robot(i).ag_vision_box_y_plus_one=ori_saved_box;
    % ori_saved_box=extractbox(ag_three_dim, box_size_define, current.x_loc,current.y_loc,(current.z_loc-1),zero_size);
    % robot(i).ag_vision_box_z_minus_one=ori_saved_box;
    % ori_saved_box=extractbox(ag_three_dim, box_size_define, current.x_loc,current.y_loc,(current.z_loc+1),zero_size);
    % robot(i).ag_vision_box_z_plus_one=ori_saved_box;
    
    
    %Use this robot location minus previous robot location
    parent=t.get(t.getparent(n(node_ind)));
    prev_x=current.x_loc-parent.x_loc;
    prev_y=current.y_loc-parent.y_loc;
    prev_z=current.z_loc-parent.z_loc;
    prev_magnitude=sqrt(prev_x*prev_x+prev_y*prev_y+prev_z*prev_z);
    prev_x_direction=prev_x/prev_magnitude;
    prev_y_direction=prev_y/prev_magnitude;
    prev_z_direction=prev_z/prev_magnitude;
    robot(i).prev_x_dir=abs(prev_x_direction);
    robot(i).prev_y_dir=abs(prev_y_direction);
    robot(i).prev_z_dir=abs(prev_z_direction);
    %initialize the direction of x y z
    robot(i).see_x_dir=(prev_x_direction);
    robot(i).see_y_dir=(prev_y_direction);
    robot(i).see_z_dir=(prev_z_direction);
    
    
    
    robot(i).prev_mag=prev_magnitude;
    %this is just for bebug
    %currenti=current.index_ori;
    node_ind_next=t.getchildren(n(node_ind))-1;
    size_children=numel(node_ind_next);
    
    %% Just define fissure
    %%
    if size_children==1
        robot(i).fissure=0;
    else
        robot(i).fissure=1;
    end
    if size_children==1
        next=t.get(n(node_ind_next));
        line([current.x_loc, next.x_loc], [current.y_loc,next.y_loc], [current.z_loc, next.z_loc]);
        
        %start to define midrobot
        robot(i).mid_robot.x_loc=(current.x_loc+next.x_loc)/2;
        robot(i).mid_robot.y_loc=(current.y_loc+next.y_loc)/2;
        robot(i).mid_robot.z_loc=(current.z_loc+next.z_loc)/2;
        
        
        
        %next direction
        next_x=next.x_loc-current.x_loc;
        next_y=next.y_loc-current.y_loc;
        next_z=next.z_loc-current.z_loc;
        next_magnitude=sqrt(next_x*next_x+next_y*next_y+next_z*next_z);
        next_x_direction=next_x/next_magnitude;
        next_y_direction=next_y/next_magnitude;
        next_z_direction=next_z/next_magnitude;
        robot(i).next_x_dir=abs(next_x_direction);
        robot(i).next_y_dir=abs(next_y_direction);
        robot(i).next_z_dir=abs(next_z_direction);
        robot(i).next_mag=next_magnitude;
        robot(i).radius=current.radius;
        %pause(0.3)
        
        %mid robot
        % robot(i).mid_robot.next_x_dir=(robot(i).next_x_dir+robot(i).prev_x_dir)/2;
        % robot(i).mid_robot.next_y_dir=(robot(i).next_y_dir+robot(i).prev_y_dir)/2;
        % robot(i).mid_robot.next_z_dir=(robot(i).next_z_dir+robot(i).prev_z_dir)/2;
        tmp=normr([abs(abs(next_x)+robot(i).prev_mag*robot(i).prev_x_dir)/2,abs(abs(next_y)+robot(i).prev_mag*robot(i).prev_y_dir)/2,abs(abs(next_z)+robot(i).prev_mag*robot(i).prev_x_dir)/2]);
        robot(i).mid_robot.next_x_dir=tmp(1);
        robot(i).mid_robot.next_y_dir=tmp(2);
        robot(i).mid_robot.next_z_dir=tmp(3);
        robot(i).mid_robot.next_mg=norm([[abs(abs(next_x)+robot(i).prev_mag*robot(i).prev_x_dir)/2,abs(abs(next_y)+robot(i).prev_mag*robot(i).prev_y_dir)/2,abs(abs(next_z)+robot(i).prev_mag*robot(i).prev_x_dir)/2]],2);
        
    elseif size_children==2
        val=node_ind_next(1);
        next=t.get(n(val));
        %this is just for debug
        %next.index_ori;
        next_x=next.x_loc-current.x_loc;
        next_y=next.y_loc-current.y_loc;
        next_z=next.z_loc-current.z_loc;
        next_magnitude=sqrt(next_x*next_x+next_y*next_y+next_z*next_z);
        next_x_direction=next_x/next_magnitude;
        next_y_direction=next_y/next_magnitude;
        next_z_direction=next_z/next_magnitude;
        robot(i).next_x_dir=abs(next_x_direction);
        robot(i).next_y_dir=abs(next_y_direction);
        robot(i).next_z_dir=abs(next_z_direction);
        
        
        
        
        robot(i).next_mag=next_magnitude;
        
        line([current.x_loc, next.x_loc], [current.y_loc,next.y_loc], [current.z_loc, next.z_loc]);
        %pause(0.3)
        val=node_ind_next(2);
        
        next=t.get(n(val));
        
        next_x=next.x_loc-current.x_loc;
        next_y=next.y_loc-current.y_loc;
        next_z=next.z_loc-current.z_loc;
        next_magnitude=sqrt(next_x*next_x+next_y*next_y+next_z*next_z);
        next_x_direction=next_x/next_magnitude;
        next_y_direction=next_y/next_magnitude;
        next_z_direction=next_z/next_magnitude;
        robot(i).next_two_x_dir=next_x_direction;
        robot(i).next_two_y_dir=next_y_direction;
        robot(i).next_two_z_dir=next_z_direction;
        robot(i).next_two_mag=next_magnitude;
        
        %this is just for debug
        %next.index_ori;
        line([current.x_loc, next.x_loc], [current.y_loc,next.y_loc], [current.z_loc, next.z_loc]);
        %pause(0.3)
    end
end



%% start to extract emuler angle
for   i=1:numel(parind)-1
    
    [robot(i).prev_alpha,robot(i).prev_beta,r_one]= cart2sph(robot(i).prev_x_dir,robot(i).prev_y_dir,robot(i).prev_z_dir);
    %r_one
    [robot(i).next_alpha,robot(i).next_beta,r_one]= cart2sph(robot(i).next_x_dir,robot(i).next_y_dir,robot(i).next_z_dir);
    %r_one
    if robot(i).fissure==1
        [robot(i).next_two_alpha,robot(i).next_two_beta]=emulerangle(robot(i).next_two_x_dir,robot(i).next_two_y_dir,robot(i).next_two_z_dir);
    end
    
    
    if robot(i).fissure==0&&(i>1)
        %first parameter done
        ori_saved_box=extractbox(three_dim, box_size_define, robot(i).mid_robot.x_loc,robot(i).mid_robot.y_loc,robot(i).mid_robot.z_loc,zero_size);
        robot(i).mid_robot.ori_vision_box=ori_saved_box;
        
        ori_saved_box=extractbox(salt_pepper_three_dim, box_size_define, robot(i).mid_robot.x_loc,robot(i).mid_robot.y_loc,robot(i).mid_robot.z_loc,zero_size);
        %second parameter done
        robot(i).mid_robot.sp_vision_box=ori_saved_box;
        
        ori_saved_box=extractbox(ag_three_dim, box_size_define, robot(i).mid_robot.x_loc,robot(i).mid_robot.y_loc,robot(i).mid_robot.z_loc,zero_size);
        %third parameter done
        robot(i).mid_robot.ag_vision_box=ori_saved_box;
    end
end






