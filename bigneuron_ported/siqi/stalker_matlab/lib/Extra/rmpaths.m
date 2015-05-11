function rmaths(X)
%
% RMPATHS    Remove directory to search path (accepts cells and matrix)
%
%    RMPATHS(path) is the same as RMPATH(path) except that is accepts cell 
%    arrays and string matrix as inputs.
%
%    If used without arguments, RMPATHS removes the current directory from the path.
%

switch nargin
case 0
   rmpath(cd)   
case 1
   if ~iscell(X)
      X = cellstr(X);
   end
   rmpath(X{:})      
   
otherwise
   error('Too many parameters')
end
