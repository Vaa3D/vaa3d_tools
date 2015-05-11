function N = COUNT(X)
%
% COUNT   Counts the number of different elements.
%
%    N = COUNT(X) counts the number of differet elements present
%    in the matrix X.
%

if ~class(X,'double')
   error('COUNT uses the SORT function which is not defined for variable of class uint8');
end

if prod(size(X)) == 0;     % if X is empty
   N=0;
   return
elseif prod(size(X)) == 1; % if X is 1x1
   N=1;
   return
end

X = sort(X(:));
N = nnz(diff(X))+1;
   