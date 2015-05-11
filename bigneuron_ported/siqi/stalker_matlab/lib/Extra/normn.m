function Y = normn(X,P,DIM)
% NORMN   Matrix norm.
%
%   Y = NORMN(X,P,DIM) returns the P-norm of matrix X taken in 
%   dimension DIM. P can be any number or 'inf'.
%
%   Example: in a 3D matrix U (size(U)=AxBxC) , y = NORMN(u,2,3)
%   will return a AxB matrix which elements are the 2-norm of the
%   1xC vectors in the 3rd dimension of u.
%

siz = size(X);
siz(DIM) = 1;
Y = zeros(siz);
if strcmp(P,'inf')
   Y = max(Y,max(X,[],DIM));
elseif P==1
   Y = sum(X,DIM);
else
   Y = sum(X.^P,DIM);
   Y = Y.^(1/P);
end
