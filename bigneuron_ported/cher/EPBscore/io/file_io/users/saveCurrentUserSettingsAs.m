function varargout=saveCurrentUserSettingsAs
	global state

	if ~isempty(state.userSettingsPath)
		cd(state.userSettingsPath);
	end

	[fname, pname]=uiputfile('*.usr', 'Choose user settings file');

	if ~isnumeric(fname)
		periods=findstr(fname, '.');
		if any(periods)								
			fname=fname(1:periods(1)-1);
		end	
		state.userSettingsName=fname;
		state.userSettingsPath=pname;
		saveCurrentUserSettings;
	end
