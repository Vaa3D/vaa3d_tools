function [Y, varargout] = shrink(X,E)
%
% SHRINK    Removes outer zeros of a matrix.
%
%   Y = SHRINK(X) removes the outer columns/rows/etc (borders) that
%   contains only zeros on a matrix X. The shrinking goes on untill
%   the first column/row/etc containing a nonzero element is found
%
%   Examples: SHRINK([0 0 1 0 2 3 0]) = [1 0 2 3]
%
%             SHRINK([[0 0 0 0 0 0 0]  = [[0 0 1 2]
%                      0 0 0 1 2 0 0]     [0 0 0 0]
%                      0 0 0 0 0 0 0]     [3 0 0 0]]
%                      0 3 0 0 0 0 0]])
%
%  Y = SHRINK(X,E) removes columns/rows/etc containing only the 
%  element E, intead of zero.
%
%  [Y,Li,Ls] = SHRINK(...) also returns the limits of the shrinking,
%  Li = inferior limits and Ls = superior limits.
%
%    See also GROW, ROLL, PAD, PADC.
%

if nargin==1
   E = 0;
end

% If X is row vector transform to col vector (to prevent problems with find)
flagrow = 0;
if ndims(X)==2 & size(X,1)==1 & size(X,2)>=1
   X = X';
   flagrow = 1;
end

% Finding columns/rows/etc to be extracted
c = ind2subm( size(X), find(X~=E) );

% Get minimuns and maximuns founds
li = min(c,[],1)';
ls = max(c,[],1)';

% Mounting text: li(1):ls(1),li(2):ls(2)...
N = ndims(X);
for i = 1 : N
   ind{i} = li(i):ls(i);
end

% Making Y
Y = X(ind{:});

% De-transposing is X is row vector
if flagrow
   Y = Y';
end
if nargout>1
   varargout{1} = li';
   varargout{2} = ls';
end