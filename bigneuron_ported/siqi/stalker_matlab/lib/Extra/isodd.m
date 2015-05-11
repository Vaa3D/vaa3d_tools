function Y = isodd(X)
%
% ISODD     True for odd numbers
%
%    Y = ISODD(X) returns true (1) is X is odd, false (0) if X is even.
%    If X is not an integer will return 0.
%

X = X+1;
Y = logical( 2*floor(X/2)==X );
