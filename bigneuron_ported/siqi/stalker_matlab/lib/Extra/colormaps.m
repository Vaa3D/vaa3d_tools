function Y = colormaps(varargin)
% COLORMAPS   Symmetrical colormap
%
%    y = COLORMAPS(map) creates a symmetrical colormap based on the colormap "map".
%    If used without arguments, COLORMAPS will run on the current colormap.
%
%    "map" can be any valid colormap
% 

if nargin == 0
   X = colormap;
else
   X = varargin{1};
end

[r c] = size(X);

t = X(1:2:end,:);
if mod(r,2)==0
   Y = [t; flipud(t)];
else
   Y = [ t; flipud( t(1:end-1,:) )  ];
end
