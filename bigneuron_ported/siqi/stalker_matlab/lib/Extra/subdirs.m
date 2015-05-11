function Y = subdirs(varargin)
%
% SUBDIRS    Returns the subdirectories of a directory
%
%    Y = SUBDIRS(X) returns a cell array containing the names of the
%    first level subdirectories of directory 'x'.
%    
%    Y = SUBDIRS(X,L) does the same untill subdirs level L.
%    Y = SUBDIRS(X,'all') returns the subdirs at any level.
%

switch nargin
case 0
   error('Too few parameters.')
   
case 1
   
   Y = cell(0);  % To avoid not defining output variable
   DIRNAME = varargin{1};
   if isa(DIRNAME,'cell')
      N = length(DIRNAME);   % Number of dirs.
   else 
      N = 1;
      temp = DIRNAME;      % \
      clear DIRNAME        % | To allways have a cell of strings
      DIRNAME{1} = temp;   % /
   end
   
   for n = 1 : N
      clear y2
      dirn = DIRNAME{n};
      while strcmp(dirn(end),'\')   % Remove final slash, if present
         dirn = dirn(1:end-1);
      end
      
      d = dir(dirn);          % The complete directory list
      if isempty(d)
         error('Directory not found.')
         return
      end
      
      isd = cat(1,d.isdir);   % The isdir field
      
      f = find(isd==1);       
      d = struct2cell(d);
      
      f2 = strcmp(d(1,f),'.'); % Removing dir '.'
      f = f(~f2);
      f2 = strcmp(d(1,f),'..'); % Removing dir '..'
      f = f(~f2);
      y2 = d(1,f)';
      
      for i = 1 : length(f)
         y2{i} = [dirn '\' y2{i}];  % Add initial directory path and slash
      end
      
      if n == 1   % Concatenating answers
         Y = y2;
      else
         Y = [Y;y2];
      end
   end
   
   
case  2
   
   Y = subdirs(varargin{1});
   temp = Y;
   
   if strcmp(varargin{2},'all')
      while ~isempty(temp)
         temp = subdirs(temp);
         if ~isempty(temp)
            Y = [Y;temp];
         end
      end
   else
      for l = 2 : varargin{2}
         temp = subdirs(temp);
         if ~isempty(temp)
            Y = [Y;temp];
         end
      end
   end
   
otherwise
   error('Too many parameters')
end
