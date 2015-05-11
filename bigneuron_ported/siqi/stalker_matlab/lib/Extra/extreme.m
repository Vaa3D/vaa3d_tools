function varargout = extreme(X,varargin)
% EXTREME     Sets/Gets matrix extremum values.
%
%  [MIN, MAX] = EXTREME(X) return the global minimum and maximun
%  values of X.
%
%  Y = EXTREME(X,MIN,MAX) sets the minimum and maximum allowed
%  values for the elements of X. Any element bigger than MAX will
%  be set to MAX. Any element smaller than MIN will be set to MIN.
%
%  To set only the minimum or maximum replace the non constrained
%  value with []. For example, to set only the maximum use
%    Y = EXTREME(X,[],MAX)
%  
%  [Y,[NMIN NMAX]] = EXTREME(...) will also return  the number of 
%  elements affected by the minimum and the maximum constrains.
%

if nargin==1
   M(1) = min(X(:));
   M(2) = max(X(:));
   %for i = 1 : nargout
   %   varargout{i} = m{i};
   %end
   if nargout == 0
      Minimum = M(1)
      Maximum = M(2)
   else
      varargout = num2cell(M(1:nargout));
   end
   return
end

if nargin==2
   error('Incorrect number of inputs. Use [] for the unconstrained extreme.')
end

if nargin==3
   N = [0 0];
   SMIN = varargin{1}; 
   SMAX = varargin{2};
   
   if ~isempty(SMIN)
      f = find(X<SMIN);
      N(1) = length(f);
      X(f) = SMIN;
   end
   
   if ~isempty(SMAX)
      f = find(X>SMAX);
      N(2) = length(f);
      X(f) = SMAX;
   end
   
   switch nargout
   case 0
      Modifications = N
   case 1
      varargout{1} = X;
   case 2
      varargout{1} = X;
      varargout{2} = N;
   end
end
