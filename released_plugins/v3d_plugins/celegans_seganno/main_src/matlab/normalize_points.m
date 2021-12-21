%centrilize and scale the point set
%xn = T*x;
%x: every column represent a point [3*N]
%by Lei Qu
function [xn, T]=normalize_points(x)

% compute the center of every points's XYZ coordinate
x_avg=mean(x, 2);

% center the points
for i=1:size(x,1)
    xc(i,:)=x(i,:)-x_avg(i);
end

% compute the average distance of every point to the origin
p2o=sqrt(sum(xc.^2,1));%(x^2+y^2+z^2)^0.5
p2o_avg=mean(p2o);
% compute the scale factor
s=1/p2o_avg; %

% scale the points
xn=s*xc;

% compute the transformation matrix
% T=[s 0 -s*x_avg(1); 0 s -s*x_avg(2); 0 0 1];  %for 2D point ser 
T=[s 0 0 -s*x_avg(1); 0 s 0 -s*x_avg(2); 0 0 s -s*x_avg(3); 0 0 0 1];

return;