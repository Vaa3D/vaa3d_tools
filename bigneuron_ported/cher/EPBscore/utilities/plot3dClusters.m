function plot3dClusters(x, y, z, stability, varargin)
%type:  figure, plot3dclusters(x, y, z, stability, 4, 'hamming','complete')
%where hamming and complete are cluster properties(i.e. 'complete' calculates the Largest distance
maxClust = 4;
if ~isempty(varargin)
    maxClust = varargin{1};
end

variable= 'single'; % if I don't set the stability variable it uses by default 'single'another method of clustering
if length(varargin) > 2
 variable = varargin{3};  
end
distanceMetric = 'euclidean'; %methods of clustering
if length(varargin) > 1
    distanceMetric = varargin{2};
end


%Invert Z
z(find(z ~= 0)) = -1 * z(find(z ~= 0)) + max(z);  % to invert z axis and get high TB on top of z axis
% z(find(z == 0)) == Inf;  %to try to invert scale on z axis

c = ones(length(x), 3);
c(:, 1) = x;
c(:, 2) = y;
c(:, 3) = z;

linkInfo = linkage(pdist(c, 'mahalanobis'),variable); %clustering functions: linkage, pdist, cluster
clusters = cluster(linkInfo, 'MaxClust', maxClust);
% clusters(:)=1;  %to get all black circles we set only 1 cluster

h= scatter3(x, y, z, 'o', clusters, 'filled'); % simple way of scatter
% plot
%h1= scatter3(x(find(stability == 0)), y(find(stability == 0)), z(find(stability == 0)), 9, clusters(find(stability == 0)), 'filled');
%set(h1, 'MarkerSize', 6)  % stability function to get only the stable ones with black filled circles

hold on
%h2= scatter3(x(find(stability == 1)), y(find(stability == 1)), z(find(stability == 1)), 7, clusters(find(stability == 1))); %to get the dynamic ones with colored circles
%h3= scatter3(x(find(stability == 1)), y(find(stability == 1)), z(find(stability == 1)), 7, clusters(find(stability == 1)), 'filled'); % to get the dynamic ones with filled smaller black circles

% set(h2, 'Marker', '^');
hold off
%h = [h1' h2'];
xlabel('Local IVS [\mum]', 'fontweight', 'bold','fontsize', 14);
ylabel('Brightness [a.u.]','fontweight', 'bold','fontsize', 14);
zlabel('Local  ITBS [\mum]','fontweight', 'bold','fontsize', 14);
title('Cluster Analysis','fontweight', 'bold','fontsize', 14);
view(-36.5, 15);

set(h, 'MarkerSize', 6);

colormap(hsv(maxClust));

% set(h, 'MarkerFaceColor', [0 0 0], 'MarkerEdgeColor', [0 0 0], 'MarkerSize', 6);  % sets everything to black
%set(h3, 'MarkerFaceColor', [0 0 0], 'MarkerEdgeColor', [0 0 0], 'MarkerSize', 4); 
% visualises the dynamic puncta as black dots inside
% cluster circles (h2)

return;