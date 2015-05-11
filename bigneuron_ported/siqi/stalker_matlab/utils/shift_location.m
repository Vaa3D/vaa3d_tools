function [new_x, new_y, new_z]=shift_location(old_x_loc,old_y_loc,old_z_loc,x_shift,y_shift,z_shift)
new_x=old_x_loc+x_shift;
new_y=old_y_loc+y_shift;
new_z=old_z_loc+z_shift;
end