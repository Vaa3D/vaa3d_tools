function out = doesFileExist(fullfilename)
% this function returns a 1 if the file exists and a 0 if it does not
%also works for directories...

%[path,name,ext]= fileparts(fullfilename);
d = dir(fullfilename);
if isempty(d)
	out=0;
else
	out=1;
end
