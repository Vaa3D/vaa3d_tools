function Y=shift(varargin)
% SHIFT   Shifts matrix elements.
%    Y = SHIFT(X,[M1 M2 ... MN]) creates Y based on the shifitting of X by
%    Mi elements on direction i. Mi can be positive or negative integers.
%    Y is padded with to fill places left by the shifitting.
%    Y = SHIFT(X,[Mi],E) pads Y with 'E' instead of zeros>
%
%    Y = SHIFT(...,'grow') do not discard matrix elements. Instead, the matrix
%      is grown by Mi elements in each dimension to fit the newly created zeros.
%
%    If ndims(X)>1, SHIFT(X,n) == SHIFT(X,n*ones(1,ndims(X)))
%
[X,M,E,mgrow] = parse_inputs(varargin{:});

N = ndims(X);
if mgrow
   Y = E*ones(abs(M)+size(X));
   M(M<0) = 0;
   
   for i = 1 : N
      ind{i} = M(i)+1 : M(i)+size(X,i);
   end
   Y(ind{:}) = X;
     
else
   Y = E*ones(size(X));
   for i = 1 : N
      if M(i) >= 0
         indy{i} = M(i)+1 : size(Y,i);
         indx{i} = 1 : size(X,i) - M(i);
      else
         indy{i} = 1 : size(Y,i) + M(i);
         indx{i} = -M(i)+1:size(X,i);
      end
   end
   Y(indy{:}) = X(indx{:});
end



%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
function [X,M,E,mgrow] = parse_inputs(varargin)
find_par = [0 0 0 0];
mgrow = 0;

for i = 1 : nargin
   if strcmp(varargin{i},'grow')
      mgrow = 1;
      find_par(4) = 1;
   elseif find_par(1) == 0
      X = varargin{i};
      M = ones(1,ndims(X));
      E = 0;
      find_par(1) = 1;
   elseif find_par(2) == 0
      M = varargin{i};
      find_par(2) = 1;
   elseif find_par(3) == 0
      E = varargin{i};
      find_par(3) = 1;
   else
      error('Too many \ Unknown parameter.');
   end
end   

if find_par(1) == 0
   error('Too few inputs')
end

if length(M)==1
   M = M*ones(1,ndims(X));
   if sum(size(X)>1)==1 % X é vetor
      M(size(X)==1) = 0;
   end
end

if ndims(X)~=length(M) &  ~(ndims(X)==2 & length(M)==1)
   error('Incompatible dimensions.')
end
