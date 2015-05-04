function updateKeepAllSlicesCheckMark
% BSMOD - 1/1/2 - sets check mark next to keepAllSlicesInMemory selection in settings menu

    global gh state
	% get the index of the standard mode selection of the settings menu
	children=get(gh.mainControls.Settings, 'Children');			
	index=getPullDownMenuIndex(gh.mainControls.Settings, 'Keep all slices in memory');
	
    if state.internal.keepAllSlicesInMemory==0
        set(children(index), 'Checked', 'off');
		if ~state.files.autoSave
			beep;
			errordlg({ ...
				'''Keep All Slices In Memory'' is OFF and ''Auto Save'' is OFF.' , ...
				'Data will be lost for all acquisitions of more than 1 slice.', ...
				'Recommend turning ''Auto Save'' on.'}, ...
				'Warning', 0);
		end
	else
        set(children(index), 'Checked', 'on');
    end

