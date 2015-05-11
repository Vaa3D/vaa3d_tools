function [x,y,z]=rand_dir(x,y,z)
if rand>0.5
    x=-x;
end
if rand<0.5
    y=-y;
end
if rand>0.5
    z=-z;
end
end