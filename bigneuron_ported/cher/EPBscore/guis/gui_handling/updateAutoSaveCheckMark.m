function updateAutoSaveCheckMark
% BSMOD - 1/1/2 - sets check mark next to autoSave selection in settings menu

    global gh state
	% get the index of the standard mode selection of the settings menu
	children=get(gh.mainControls.Settings, 'Children');			
	index=getPullDownMenuIndex(gh.mainControls.Settings, 'Auto save');
	
    if state.files.autoSave==0
        set(children(index), 'Checked', 'off');
		hideGUI('gh.mainControls.baseName');
		hideGUI('gh.mainControls.fileCounter');
		hideGUI('gh.mainControls.baseNameLabel');
		hideGUI('gh.mainControls.fileCounterLabel');
		if ~state.internal.keepAllSlicesInMemory
			beep;
			errordlg({ ...
				'''Keep All Slices In Memory'' is OFF and ''Auto Save'' is OFF.' , ...
				'Data will be lost for all acquisitions of more than 1 slice.', ...
				'Recommend turning ''Auto Save'' on.'}, ...
				'Warning', 0);
		end

	else
        set(children(index), 'Checked', 'on');
		seeGUI('gh.mainControls.baseName');
		seeGUI('gh.mainControls.fileCounter');
		seeGUI('gh.mainControls.baseNameLabel');
		seeGUI('gh.mainControls.fileCounterLabel');
    end
        