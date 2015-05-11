function Y = cmap(varargin)
% CMAP   Creates colormap
%
%    Y = CMAP(N,C1,C2,,...,Cm) creates a colormap of size N composed
%    by interpolating the colors:
%             C1 -> C2 -> ... -> Cm
%
%    Ci is a 1x3 vector defining the RGB components of the color i
%

if  nargin < 3
   error('Too few inputs.')
end

N = varargin{1};
C = cat(1,varargin{2:end});

Y = zeros(N,3);
ls = 1;
r = N/(size(C,1)-1); % Length of each color transition.
for i = 1 : size(C,1)-1
   li = ls;
   ls = round(i*r);
   d = ls - li + 1;
   Y(li:ls,:) = [linspace(C(i,1),C(i+1,1),d); linspace(C(i,2),C(i+1,2),d); linspace(C(i,3),C(i+1,3),d)]';
end
