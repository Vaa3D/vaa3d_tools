function Y = subdim(X,N,DIM)
% SUBDIM   Extracts matrix elements by dimension
%
%   Y = SUBDIM(X,N,DIM) returns the matrix X(:,:,...,N,...,:)
%   where the number N is at position DIM.
%
%   For example, to extract the submatrix X(:,:,2) of a 3D matrix
%   X use -> Y = SUBDIM(X,2,3).
%
%   N can be a vector: Y = SUBDIM(X,[1 3 5],DIM) or Y = SUBDIM(X,3:6,DIM)
%

for i = 1 : ndims(X)
   if i == DIM
      ind{i} = N;
   else
      ind{i} = 1:size(X,i);
   end
end

Y = X(ind{:});