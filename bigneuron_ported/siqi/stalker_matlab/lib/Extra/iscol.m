function Y = iscol(X)
%
% ISCOL    True for column vectors.
%
%   Y = ISCOL(X) returns logical 1 if X is a column vector, 0 otherwise.
%   ISCOL returns 1 for scalars also.
%
%    See also: ISROW.
%

if ndims(X)==2 & size(X,1)>=1 & size(X,2)==1
   Y = logical(1);
else
   Y = logical(0);
end
