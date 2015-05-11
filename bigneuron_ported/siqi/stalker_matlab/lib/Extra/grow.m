function Y = grow(varargin)
%
% GROW    Expands a matrix.
%
%   Y = GROW(X,[M1 M2 ... Mn]) creates a matrix 'Y' by the expansion
%   of the matrix 'x' of Mi elements on directions i e -i. The size 
%   of the new matrix 'Y' will be:
%       size(Y) = size(x) + 2*[M]
%
%   Example: X = [ 1 2 3; 4 5 6; 7 8 9]
%   Y = GROW(X,[1 0])
%   Y = [1 2 3; 1 2 3; 4 5 6; 7 8 9; 7 8 9]
%
%   Y = GROW(X,[1 1])
%   Y = [1 1 2 3 3; 1 1 2 3 3; 4 4 5 6 6; 7 7 8 9 9; 7 7 8 9 9]
%
%   Y = GROW(X) uses as standard Mi = 1.
%
%    If DIM(X)>1, GROW(X,N) = GROW(X,N*ones(DIM(x)))
%
%   Y = GROW(X,[M], 'a') performs assimetric expansion. If Mi is even
%   assimetric expansion is the same as simmetric expansion, execpt that
%   the expansion in the assimetric mode will be half of the one in the
%   simmetric mode. If Mi is odd, the matrix will be expanded ceil(Mi/2)
%   elements in the -i direction and floor(Mi/2) in the i. The size of
%   the expanded matrix will be:  
%       size(y) = size(X) + [M]
%
%  [M] can contain negative integers. In this case, the outer elements
%  of X will be discarded.
%
%    See also ROLL, PAD, PADC, SHRINK.
%


[X, M, assimetric] = parse_inputs(varargin{:});

ind = cell(length(M),1);

if assimetric == 0
   for i = 1 : length(M)
      if M(i) >= 0
         ind{i} = [repmat(1,[1 M(i)]) 1:size(X,i) repmat(size(X,i),[1 M(i)]) ];
      else
         ind{i} = 1+(-M(i)) : size(X,i)-(-M(i)); % using -M(i) to change sign
      end
   end
else % if not assimetric
   for i = 1 : length(M)
      if M(i) >= 0
         ind{i} = [repmat(1,[1 ceil(M(i)/2)]) 1:size(X,i) repmat(size(X,i),[1 floor(M(i)/2)]) ];
      else
         ind{i} = 1+ceil(-M(i)/2) : size(X,i) - floor(-M(i)/2);
      end
   end
end
   
Y = X(ind{:});


%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
function [X,M,assimetric] = parse_inputs(varargin)
assimetric = 0;

switch nargin
case 0
   error('Too few inputs!')
   return
   
case 1
   X = varargin{1};
   M = ones(1,ndims(X));
   
case 2
   X = varargin{1};
   M = varargin{2};
   if strcmp(M,'a')
      M = ones(1,ndims(X));
      assimetric = 1;
   end
   
case 3
   X = varargin{1};
   M = varargin{2};
   if ~strcmp('a', lower(varargin{3}))
      error('Unknown parameter.');
   end
   assimetric = 1;
  
case 4
   error('Too many inputs!')
   return
end

if length(M)==1 
   if sum(size(X)>1)>1
      M = M*ones(1,ndims(X));
   elseif ndims(X)==2 & size(X,1)==1 & size(X,2)>1
      M = [0 M];
   elseif ndims(X)==2 & size(X,1)>1 & size(X,2)==1
      M = [M 0];
   end
end

if length(M)~=ndims(X)
   error('Invalid dimensions')
end

