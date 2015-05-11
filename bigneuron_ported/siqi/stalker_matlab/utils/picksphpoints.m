function [th, phi] = picksphpoints(ndir)
% Uniformly pick directions on a unit sphere
% Ref: http://mathworld.wolfram.com/SpherePointPicking.html

nphi = int16((ndir/2)^0.5);
nth = 2 * nphi;

u = linspace(0, 1, nth);
v = linspace(0, 1, nphi);

u = repmat(u, nphi, 1);
u = u(:);

v = repmat(v, nth, 1);
v = v';
v = v(:);

%u = rand(1, ndir);
%v = rand(1, ndir);

th = 2 * pi * u;
phi = acos(2 * v - 1);

disp([th; phi]')

end