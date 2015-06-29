close all
filepath = fileparts(mfilename('fullpath'));

% Plot Seeds
figure
title('Checking Seeds')
seeds = csvread(fullfile(filepath, 'seeds.csv'));
seeds(isnan(seeds)) = 0 ;
bimg = csvread(fullfile(filepath, 'binaryimg.csv'));
hold on
plot3(bimg(:,1), bimg(:,2), bimg(:,3), 'b.');
plot3(seeds(:,1), seeds(:,2), seeds(:,3), 'r+');
hold off

nseed = size(seeds, 1);
% s
% for i = 1 : nseed
% 	sph = csvread(fullfile(filepath, sprintf('sampledsphere%d.csv', i-1)));
% 	momentlist = csvread(fullfile(filepath, sprintf('moment%d.csv', i-1)));
% 	[value index] = min(momentlist(:,3));
% 	seedx = repmat(seeds(i, 1), size(sph, 1), 1);
% 	seedy = repmat(seeds(i, 2), size(sph, 1), 1);
% 	seedz = repmat(seeds(i, 3), size(sph, 1), 1);
% 	seedx = seedx(index);
% 	seedy = seedy(index);
% 	seedz = seedz(index);
% 	index
% 	sph(index, 1)
% 	sph(index, 2)
% 	sph(index, 3)
% 	quiver3(seedx, seedy, seedz, sph(index, 1), sph(index, 2), sph(index, 3), 20);
% end

figure
halfsphere = csvread(fullfile(filepath, 'halfsphere.csv'));
scatter3(halfsphere(:,1), halfsphere(:, 2), halfsphere(:, 3))
title('Checking halfsphere')



%Show spheres
% for i = 1 : nseed 
% 	figure(i + 1)
% 	view([2^0.5, 2^0.5, 2^0.5])
% 	titlestr = sprintf('sphere %d', i - 1);
% 	title(titlestr)
% 	sph = csvread(fullfile(filepath, sprintf('sampledsphere%d.csv', i-1)));
% 	peak = csvread(fullfile(filepath, sprintf('sphpeak%d.csv', i-1)));
% 	startpts = zeros(size(peak, 1), 1);
% 	hold on
% 	plot3(sph(:,1),sph(:,2),sph(:,3), 'co');
% 	%quiver3(startpts, startpts, startpts, peak(:,1), peak(:,2), peak(:,3), 4);
% end

% Show GVF
gvf = csvread(fullfile(filepath, 'gvf-littlesoma.csv'));

figure
title('GVF')
%idx = [1:100:size(gvf, 1)];

%quiver3(gvf(idx, 1), gvf(idx, 2), gvf(idx, 3), gvf(idx, 4), gvf(idx, 5), gvf(idx, 6), 2)
hold on
idx = bimg(:, 1) * max(gvf(:, 2)) * max(gvf(:, 3)) + bimg(:,2) * max(gvf(:, 3)) + bimg(:, 3);
plot3(bimg(:,1), bimg(:,2), bimg(:,3), 'rh');
quiver3(gvf(bimg(:,end), 1), gvf(bimg(:,end), 2), gvf(bimg(:, end), 3), gvf(bimg(:,end), 4), gvf(bimg(:,end), 5), gvf(bimg(:,end), 6), 0.5)
hold off

% figure
% hold on
% plot3(bimg(:,1), bimg(:,2), bimg(:,3), 'b.');
% peakdir = csvread(fullfile(filepath, sprintf('sphpeak%d.csv', nseed-1)));
% for i = 1: nseed
% quiver3(seeds(i, 1), seeds(i, 2), seeds(i, 3), peakdir(i, 1), peakdir(i, 2), peakdir(i, 3), 4);
% end
% hold off


figure
title('NextMove')
hold on
plot3(bimg(:,1), bimg(:,2), bimg(:,3), 'b.');
mp = csvread(fullfile(filepath, 'manypoint.csv'));
plot3(mp(:,1), mp(:,2), mp(:,3), 'g+');
hold off

[x,y,z] = sphere();
figure
surf(x,y,z)  % sphere centered at origin
hold on
for i = 1 : size(mp, 1)
surf(mp(i, 4) * x + mp(i, 1), mp(i, 4) * y + mp(i, 2), mp(i, 4) * z + mp(i, 3))  % sphere centered at (3,-2,0)
end
daspect([1 1 1])
hold off


addpath(genpath('/home/donghao/Downloads/TREES1.15'))
figure
tree = MST_tree (1, [mp(50, 1); mp(:, 1)], [mp(50, 2); mp(:, 2)], [mp(50, 3); mp(:, 3)], .5, 50, [], [], 'none');
plot_tree(tree)
