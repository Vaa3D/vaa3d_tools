function Y = toggle(varargin)
% TOGGLE   Matrix elements classification.
%
%   Y = TOGGLE(X,X1,X2) generates the binary matrix Y by comparing
%   each element of the matrix X with X1 and X2. If the element of
%   X is closer to the element of X1, the corresponding element of
%   Y will be zero. Otherwise if the element of X is closer to the
%   one in X2, the corresponding Y element will be 1.
%
%   Y = TOGGLE(X,X1,X2,'s') substitutes the output zeros for the 
%   coresponding  elements of  X1 and the output  ones  for  the 
%   correspondig elements of X2.
%
%   X1 and X2 must be the same size as X or scalars.
%   If X, X1 and X2 are uint8, Y is uint8. Otherwise Y is double.
%

[X,X1,X2,S] = parse_inputs(varargin{:});

flag = 0;
if ~isa(X1,'double')
   X1 = double(X1);
   flag = flag + 1;
end
if ~isa(X2,'double')
   X2 = double(X2);
   flag = flag + 1;
end

Y = zeros(size(X));

d = abs(X-X1) > abs(X-X2);

if S==0
   Y(d) = 1;
else
   Y(d) = X2(d);
   Y(~d) = X1(~d);
end

if flag==0
   Y = uint8(Y);
end

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
function [X,X1,X2,S] = parse_inputs(varargin)
switch nargin
case {0,1,2}
   error('Too few inputs')
   
%case 1
%   error('Missing variable or function')
   
%case 2
%   error('Missing variable or function')
   
case 3
   X = varargin{1};
   X1 = varargin{2};
   X2 = varargin{3};
   S = 0;
   
case 4
   X = varargin{1};
   X1 = varargin{2};
   X2 = varargin{3};
   if strcmp('s',varargin{4})
      S = 1;
   else
      error('Unknown parameter')
   end
       
case 5 
   error('Too many inputs')
      
end
   

if size(X) ~= size(X1) 
   if length(X1)==1
      X1 = X1*ones(size(X));
   else
      error('X1 must be the same size as X, or scalar');
   end
end
if size(X) ~= size(X2) 
   if length(X2)==1
      X2 = X2*ones(size(X));
   else
      error('X2 must be the same size as X, or scalar');
   end
end