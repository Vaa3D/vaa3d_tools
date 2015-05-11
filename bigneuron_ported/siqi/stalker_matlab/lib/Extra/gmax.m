function varargout = gmax(X)
% GMAX   Global maximum.
%
%  Y = GMAX(X) return the global maximum value of X.
%
%  [Y,POS] = GMAX(X) returns the value and position of the
%  global maximum. If X has more than one global maximum only
%  the position of the first will be returned.
%

if nargout < 2
   varargout{1} = max(X(:));
   
elseif nargout == 2
   [varargout{1},POS] = max(X(:));
   
   POS = ind2subm(size(X),POS);
   
   varargout{2} = POS;
else
   error('Too many outputs.')
end

