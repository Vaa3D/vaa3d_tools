function Y = resize(varargin)
%  RESIZE    Resizes an matrix.
%
%     Y = RESIZE(X,[R1 R2 ... Rn]) creates the matrix Y containing the elements of X
%     which are at indexes m*Ri+1 (m = 0,1,2,..) at dimension i. 
%     Y = RESIZE(X, R) is the same as Y = RESIZE(X,[R R ... R]).
%
%     Y = RESIZE(X,[L1 L2 ... Ln],'size') creates the image Y with Li elements on 
%     dimension i. 
%
%     ATENTION : No interpolation is done by this function. It only works with
%                the matrix indexes.
%
%     See also : RESIZE, RESIZE2

[X, R, FIXSIZE] = parse_inputs(varargin{:});

% Creates indexes list
if FIXSIZE
   for dim = 1 : length(R)
      ind{dim} = round( linspace(1,size(X,dim),R(dim)) );
   end
else
   for dim = 1 : length(R)
      ind{dim} = round(1 : R(dim) : size(X,dim));
   end
end

Y = X(ind{:});

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
function [X, R, FIXSIZE] = parse_inputs(varargin)

if nargin < 2
   error('Too few inputs.')
elseif nargin == 2
   X = varargin{1};
   R = varargin{2};
   FIXSIZE = 0;
elseif nargin == 3;
   X = varargin{1};
   R = varargin{2};
   FIXSIZE = 0;
   if strcmp(varargin{3},'size')
      FIXSIZE = 1;
   else
      error('Unknown parameter.')
   end
else
   error('Too many inputs.')
end

if length(R) == 1 & ndims(X) > 2
   R(2:ndims(X)) = R(1);
end

if length(R)==1 & ndims(X)==2
   if size(X,1)==1 & size(X,2)~=1
      R = [1 R];
   elseif size(X,1)~=1 & size(X,2)==1
      R = [R 1];
   else
      R(2:ndims(X)) = R(1);   
   end
end

if ndims(X) > length(R)
   error('Use length(R)=1 or length(R)>=ndims(X).')
end

if FIXSIZE == 1
   if any(mod(R,1))
      error('L must contain only integers.')
   end
end