function [th, phi, r] = cart2sph_sq(x, y, z)
% Convert cartisian coordinates to spherical coordinates
% Using the mathmatician definition of the S^2
%                                2*pi > th > 0
%                                pi > phi > 0

r = norm([x, y, z]);
th = atan(y ./ x);
phi = acos(z ./ r);
th(th<0) = 2*pi + th(th<0);

end