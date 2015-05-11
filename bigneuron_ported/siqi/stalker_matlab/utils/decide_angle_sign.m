%try to decide the sign of the robot 
function [cur_x_dir, cur_y_dir]=decide_angle_sign(pre_x_dir, pre_y_dir, cur_x_dir, cur_y_dir)
decision_value(1)=(pre_x_dir-cur_x_dir)*(pre_x_dir-cur_x_dir)+(pre_y_dir+cur_y_dir)*(pre_y_dir+cur_y_dir);
decision_value(2)=(pre_x_dir+cur_x_dir)*(pre_x_dir+cur_x_dir)+(pre_y_dir+cur_y_dir)*(pre_y_dir+cur_y_dir);

[no_use index]=min(decision_value);
if  index==2
  cur_y_dir=-cur_y_dir;cur_x_dir=-cur_x_dir;
end

end