function Y = iseven(X)
%
% ISEVEN    True for even numbers
%
%    Y = ISEVEN(X) returns true (1) is X is even, false (0) if X is odd.
%    If X is not an integer will return 0.
%

Y = logical( 2*floor(X/2)==X );