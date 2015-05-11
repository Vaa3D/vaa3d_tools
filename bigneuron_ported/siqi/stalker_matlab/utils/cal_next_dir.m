function [next_x_dir,next_y_dir,next_z_dir,next_mg] =cal_next_dir(cur_x_loc,...
                                                                  cur_y_loc,...
                                                                  cur_z_loc,...
                                                                  next_x_loc,...
                                                                  next_y_loc,...
                                                                  next_z_loc)
%                                                            
                                                              
tmp=normr([next_x_loc-cur_x_loc,next_y_loc-cur_y_loc,next_z_loc-cur_z_loc]);
next_x_dir=abs(tmp(1)); 
next_y_dir=abs(tmp(2)); 
next_z_dir=abs(tmp(3));
next_mg=norm([next_x_loc-cur_x_loc,next_y_loc-cur_y_loc,next_z_loc-cur_z_loc]);

end


%try to decide the sign of the robot 
function [cur_x_dir, cur_y_dir, cur_z_dir]=decide_sign(pre_x_dir, pre_y_dir, pre_z_dir, cur_x_dir, cur_y_dir, cur_z_dir)
decision_value(1)=(pre_x_dir-cur_x_dir)*(pre_x_dir-cur_x_dir)+(pre_y_dir+cur_y_dir)*(pre_y_dir+cur_y_dir)+(pre_z_dir+cur_z_dir)*(pre_z_dir+cur_z_dir);
decision_value(2)=(pre_x_dir+cur_x_dir)*(pre_x_dir+cur_x_dir)+(pre_y_dir+cur_y_dir)*(pre_y_dir+cur_y_dir)+(pre_z_dir+cur_z_dir)*(pre_z_dir+cur_z_dir);
decision_value(3)=(pre_x_dir-cur_x_dir)*(pre_x_dir-cur_x_dir)+(pre_y_dir+cur_y_dir)*(pre_y_dir+cur_y_dir)+(pre_z_dir-cur_z_dir)*(pre_z_dir-cur_z_dir);
decision_value(4)=(pre_x_dir+cur_x_dir)*(pre_x_dir+cur_x_dir)+(pre_y_dir+cur_y_dir)*(pre_y_dir+cur_y_dir)+(pre_z_dir-cur_z_dir)*(pre_z_dir-cur_z_dir);
decision_value(5)=(pre_x_dir-cur_x_dir)*(pre_x_dir-cur_x_dir)+(pre_y_dir-cur_y_dir)*(pre_y_dir-cur_y_dir)+(pre_z_dir+cur_z_dir)*(pre_z_dir+cur_z_dir);
decision_value(6)=(pre_x_dir+cur_x_dir)*(pre_x_dir+cur_x_dir)+(pre_y_dir-cur_y_dir)*(pre_y_dir-cur_y_dir)+(pre_z_dir+cur_z_dir)*(pre_z_dir+cur_z_dir);
decision_value(7)=(pre_x_dir-cur_x_dir)*(pre_x_dir-cur_x_dir)+(pre_y_dir-cur_y_dir)*(pre_y_dir-cur_y_dir)+(pre_z_dir-cur_z_dir)*(pre_z_dir-cur_z_dir);
decision_value(8)=(pre_x_dir+cur_x_dir)*(pre_x_dir+cur_x_dir)+(pre_y_dir-cur_y_dir)*(pre_y_dir-cur_y_dir)+(pre_z_dir-cur_z_dir)*(pre_z_dir-cur_z_dir);
[no_use index]=min(decision_value);
switch index
        case 1
                             cur_y_dir=-cur_y_dir;cur_z_dir=-cur_z_dir;
        case 2
        cur_x_dir=-cur_x_dir;cur_y_dir=-cur_y_dir;cur_z_dir=-cur_z_dir;
        case 3
                             cur_y_dir=-cur_y_dir;
        case 4
        cur_x_dir=-cur_x_dir;cur_y_dir=-cur_y_dir;
        case 5
                                                 cur_z_dir=-cur_z_dir;
        case 6
        cur_x_dir=-cur_x_dir;                    cur_z_dir=-cur_z_dir;
        
        case 8
        cur_x_dir=-cur_x_dir;
end
end