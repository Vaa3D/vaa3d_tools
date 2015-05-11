function Y =addpaths(varargin)
%
% ADDPATHS    Adds directory to search path (accepts cells and matrix)
%
%    ADDPATHS(PATH) is the same as ADDPATH(PATH) except that is accepts cell 
%    arrays and string matrix as inputs.
%
%    ADDPATHS(PATH,POS) where POS is 'end' or 'begin' is valid.
%
%    If used without arguments, ADDPATHS adds the current directory to the path.
%

switch nargin
case 0
   addpath(cd)
   return
case 1
   POS = '-begin';
case 2
   POS = ['-' varargin{2}];
otherwise
   error('Too many parameters')   
end

if ~strcmp(POS,'-begin') & ~strcmp(POS,'-end')
   error('Unknown position parameter.')
   return
end

X = varargin{1};
if ~iscell(X)
   X = cellstr(X);
end

addpath(X{:},POS);   

   
