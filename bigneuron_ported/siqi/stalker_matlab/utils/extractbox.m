function saved_box=extractbox(ori_box, size, x,y,z,zero_size)
% to make sure every index is an integer
size_box=floor(size/2);
test_location(1)=floor(x+zero_size);
test_location(2)=floor(y+zero_size);
test_location(3)=floor(z+zero_size);
% drawCube([test_location(1) test_location(2) test_location(3) (size+1)])

% pause(0.1)
% x_min=test_location(1)-size_box;%-zero_size;
% x_max=test_location(1)+size_box;%-zero_size;
% y_min=test_location(2)-size_box;%-zero_size;
% y_max=test_location(2)+size_box;%-zero_size;
% z_min=test_location(3)-size_box+6;%-zero_size;
% z_max=test_location(3)+size_box;%-zero_size;
% %drawBox3d([x_min,x_max,y_min,y_max,z_min,z_max]);
% pause(0.1)
saved_box=ori_box(test_location(1)-size_box:test_location(1)...
    +size_box,test_location(2)-size_box:test_location(2)+...
    size_box,test_location(3)-size_box:test_location(3)+size_box);
end