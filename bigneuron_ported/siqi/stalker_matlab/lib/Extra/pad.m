function Y=pad(varargin)
% PAD   Pads matrix.
%
%    Y = PAD(X,[Mi]) creates the matrix Y by expanding X and and
%    padding the new elements with zeros. X will be expanded Mi
%    elements in the directions -i and +i so that:
%                  size(Y) = size(X) + 2*[Mi]
%
%    If Mi is scalar the same value will be used in all directions.
%    
%    Y = PAD(...,'e',E) will pad X with element E instead of zero.
%
%    Y = PAD(X,[Mi],'a') performs an assimetric padding. If Mi is
%    even assimetric padding will be the same as simmetric padding,
%    except that the expansion in assim. mode will be half the one
%    in simmetric mode. If Mi is odd, X will be expanded floor(Mi/2)
%    elements in direction -i and ceil(Mi/2) elements in direction
%    i. The size of Y in the assimetric mode will then be:
%                 size(Y) = size(X) + [Mi]
%
%    Y = PAD(X,[Mi],[Ni]) will pad the matrix with Mi elements in 
%    direction i and with Ni elements in direction -i.
%
%    Y = PAD(X,[Si], 'size') will pad X so that size(Y) = [Si].
%    It`s necessary that [Si] > size(Y).
%

[X,M,N,E] = parse_inputs(varargin{:});

Y = E*ones(size(X)+M+N);

for i = 1 : length(M)
   ind{i} = N(i)+1:N(i)+size(X,i);
end
Y(ind{:}) = X;

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
function [X,M,N,E,S] = parse_inputs(varargin)
if nargin==0
   error('Too few inputs!')
end

E = 0;
X = varargin{1};
if nargin > 1
   M = varargin{2};
else
   M = 1;
end
N = M;

flagN = 0;
flagA = 0;
Epos = -1;
for i = 3 : nargin
   flag = 0;
   if i == Epos
      flag = 1;
   elseif strcmp(varargin{i},'a')
      if flagN
         error('Assimetric mode cannot be used together with [Mi] and [Ni].')
      end
      M = ceil(M/2);
      N = floor(N/2);
      flag = 1;
      flagA = 1;
   elseif strcmp(varargin{i},'e')
      E = varargin{i+1};
      flag = 1;
      Epos = i+1;
   elseif strcmp(varargin{i},'size')
      M = ceil( (varargin{2}-size(X))/2 );
      N = floor( (varargin{2}-size(X))/2 );
      if any(M<0) | any(N<0)
         error('Cannot specify a size smaller than size(X).')
      end
      flag = 1;
      flagN = 1;
      if flagA
         error('Cannot use assimetric mode and fix size.')
      end
   elseif flag == 0 & flagN == 0
      N = varargin{i};
      flagN = 1;
   else
      error('Too many inputs.')
   end
end

if length(M)==1
   M = M*ones(1,ndims(X));
end
if length(N)==1
   N = N*ones(1,ndims(X));
end
if length(M)~=ndims(X) | length(N)~=ndims(X)
   error('[Mi],[Ni] and [Si] must have length equal to ndims(X).')
end
if length(E)~=1
   error('`E` must be a number.')
end