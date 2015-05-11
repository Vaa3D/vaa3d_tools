function y = scale(varargin)
% SCALE  Scales matrix elements to a new range.
%
%   y = SCALE(x,[min max]) scales the elements of matrix x to a new range
%   defined by [min, max].
%   
%   y = SCALE(x) uses the default range = [0 1]
%

[x,Imin,Imax] = parse_inputs(varargin{:});

x = double(x);
M = max(x(:));
m = min(x(:));

y = ( (x-m)/(M-m) * (Imax-Imin) ) + Imin;


function [x,Imin,Imax] = parse_inputs(varargin)
switch nargin
case 0
   error('Missing variable or function')
   
case 1
   x = varargin{1};
   Imax = 1;
   Imin = 0;
   
case 2
   x = varargin{1};
   I = varargin{2};
   if size(I,1) ~= 2 & size(I,2) ~= 2
      error('Range must be a 1x2 vector')
   end
   Imin = I(1);
   Imax = I(2);
   
case 3
   error('Too many inputs')
end
