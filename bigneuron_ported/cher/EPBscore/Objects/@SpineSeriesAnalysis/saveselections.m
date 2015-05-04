function obj=saveselections(obj)
% get the names of the images
[filename, pathname] = uiputfile(['*.sel'], 'Save Selections As');

save [pathname filename '.sel'] obj.state.display.pathnames;


% automatically generate selections