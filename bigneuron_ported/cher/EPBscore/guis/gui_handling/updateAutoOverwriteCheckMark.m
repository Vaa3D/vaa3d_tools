function updateAutoOverwriteCheckMark
% BSMOD - 1/1/2 - sets check mark next to automatic overwrite selection in settings menu

    global gh state
	children=get(gh.mainControls.Settings, 'Children');			
	index=getPullDownMenuIndex(gh.mainControls.Settings, 'Automatic overwrite');
	
    if state.files.automaticOverwrite==0
        set(children(index), 'Checked', 'off');
	else
        set(children(index), 'Checked', 'on');
		beep;
		disp('WARNING:  Automatic overwrite is on');
		disp('          No warning will be given when overwriting old data');
	end
        