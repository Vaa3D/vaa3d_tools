function out = checkFileBeforeSave(fullname)
% This function checks if the file about to be saved already exists
% name includes extension
% out = 1 indicates user fixed problem, out = 0 means they did not.

global gh state

if state.files.automaticOverwrite | ~state.files.autoSave
	out=0;
	return;
end

exist = doesFileExist(fullname);
if exist	%if the file alreayd exists
	button = questdlg('File Already Exists.  Do you wish to:', 'Overwrite warning!',...
           'Select New Basename','Overwrite', 'Cancel', 'Select New Basename');
	switch button
	case 'Overwrite'
		out =1 ;
	case 'Select New Basename'
		 answer  = inputdlg('Select base name','Choose Base Name',1,{state.files.baseName});
            if ~isempty(answer)
                state.files.baseName = answer{1};
				updateFullFileName;
                updateGUIbyglobal('state.files.baseName');
				out=1;
			else	
				out=1;	
			end
	case 'Cancel'
		out=[];
	end
else
	out=1;
end

	
