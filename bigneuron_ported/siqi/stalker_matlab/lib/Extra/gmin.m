function varargout = gmin(x)
% GMIN   Global minimum.
%
%  y = GMIN(X) return the global maximum value of X .
%
%  [Y,POS] = GMIN(X) returns the value and position of the
%  global minimum. If X has more than one global minimum only
%  the position of the first will be returned.
%

if nargout < 2
   varargout{1} = min(X(:));
   
elseif nargout == 2
   [varargout{1},POS] = min(X(:));
   
   POS = ind2subm(size(X),POS);
   
   varargout{2} = POS;
else
   error('Too many outputs.')
end


