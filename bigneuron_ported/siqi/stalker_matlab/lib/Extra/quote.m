function y = quote(varargin)
%
% QUOTE     Returns a quoted string
%
%    y = QOUTE(x) returns the string 'x' in between quotes.
%
%    Example: QOUTE('ball') -> 'ball' (The quoutes are part of the string)
%
%    This function is usefull when you must pass a parameter in quotes.
%
%    y = QUOTE(x,ASC) returns the string x in between the character char(ASC).
%    y = QUOTE(x,'c') returns the string x in between the character "c".
switch nargin
case 0
   error('Too few parameters.')
case 1
   y = strcat( char(39),varargin{1},char(39) );
case  2
   if isa(varargin{2},'char')
      y = [varargin{2}, varargin{1}, varargin{2}];
   else
      y = strcat( char(varargin{2}), varargin{1}, char(varargin{2}) );
   end
otherwise
   error('Too many parameters')
end
