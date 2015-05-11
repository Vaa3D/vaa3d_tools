function varargout = mmax(varargin)
% MMAX   Multi matrix maximum.
%
%   Y = MMAX(X1,X2,...,Xn) is the same as Y = max(X1,max(X2,...max(Xn-1,Xn)) 
%   X1,X2,...,Xn must be the same size. Y will also be the size as Xi and each 
%   element of Y is the maximum among the correspondent elements of X1,X2,...Xn.
%
%   [Y,N] = MMAX(X1,X2,...Xn) returns the maximum values and also the matrix
%   from which it came from.
%

% Input check
if nargin == 0
   error('Too few inputs.')
end
if nargout > 2
   error('Too many outputs.')
end

s = size(varargin{1});
for i = 2:nargin
   if s ~= size(varargin{i})
      error('Every input must be the same size')
   end
end

% Calculations
if nargout < 2
   Y = varargin{1};
   for i = 2 : nargin
      Y = max(Y, varargin{i});
   end
   varargout{1} = Y;
   
elseif nargout == 2
   Y = varargin{1};
   N = ones(size(Y));
   for i = 2 : nargin
      d = varargin{i}>Y;
      Y(d) = varargin{i}(d);
      N(d) = i;
   end
   varargout{1} = Y;
   varargout{2} = N;
end