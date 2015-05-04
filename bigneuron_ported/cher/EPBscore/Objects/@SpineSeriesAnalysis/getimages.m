function obj=getimages(obj)
% get the names of the images
[filenames, pathname] = uigetfile('*.tif; *.spi','Choose Files for Analysis','MultiSelect','on');
% append full path to the files
pathnames=[];
if ischar(filenames)
    filenames={filenames};
end
for i=1:size(filenames,2)
pathnames=strvcat(pathnames,[pathname char(filenames(i))]);
end
pathnames=cellstr(pathnames)';

% reverse the order
if (isempty(obj.state.display.filenames))
    obj.state.display.filenames=fliplr(flipdim(filenames,2));
    obj.state.display.pathnames=fliplr(flipdim(pathnames,2));
else
    obj.state.display.filenames=[obj.state.display.filenames fliplr(flipdim(filenames,2))];
    obj.state.display.pathnames=[obj.state.display.pathnames fliplr(flipdim(pathnames,2))];
end
cd(pathname);
set(obj.gh.mainGUI.listbox,'String',obj.state.display.pathnames);