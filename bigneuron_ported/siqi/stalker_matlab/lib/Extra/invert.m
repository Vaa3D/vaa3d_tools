function Y = invert(X)
%
% INVERT   Inverts an image.
%
%    Y = INVERT(X) inverts the colors of the image X.
%
%    If X is class uint8, INVERT is the same as  Y = 255 - X.
%    If X is double and the elements of X are inside [0 1], 
%    INVERT is the same as Y = 1 - X.
%
%    Otherwise, INVERT is the same as Y = max(X) - X + min(X).
%



if isa(X,'uint8')
   Y = uint8( 255 - double(X));
   return
end

M = max(X(:));
m = min(X(:));
if M >= 1 & m <= 0 
   Y = 1 - X;
else
   Y = M - X + m;
end

