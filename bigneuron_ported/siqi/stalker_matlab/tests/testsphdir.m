curdir = fileparts(mfilename('fullpath'));
addpath(fullfile(curdir, '..', 'utils'))

% Test picksphpoints
figure(1)
[th, phi] = picksphpoints(1000);
[x, y, z] = sph2cart_sq(th, phi, 1);
scatter3(x, y, z);

% Test dir2prob with 2 directions
gtth = [pi/3, 4*pi/3];
gtphi = [pi/3, 5*pi/6];
P = dir2prob(gtth, gtphi, th, phi, 0.3);
assert(numel(P) == numel(th));
assert((sum(P, 1) - 1.0) < eps(4), sprintf('P is not stochastic %d\n', sum(P, 1)));
disp(P)
[x, y, z] = sph2cart_sq(th, phi, P);
figure(2)
scatter3(x, y, z);

% Test dir2prob with 3 directions
gtth = [pi/2, 3*pi/2, 7*pi/4];
gtphi = [pi/2, pi/2, 3*pi/4];
P = dir2prob(gtth, gtphi, th, phi, 0.3);
assert(numel(P) == numel(th));
disp(P)
[x, y, z] = sph2cart_sq(th, phi, P);
figure(3)
scatter3(x, y, z);
