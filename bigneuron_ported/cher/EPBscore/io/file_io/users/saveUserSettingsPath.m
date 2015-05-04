function saveUserSettingsPath

	global state
	
	userPath=state.userSettingsPath;
	save(fullfile(matlabroot, 'work', 'ScanImage', ['lastUserPath.mat']), ...
		'userPath', '-mat');
	

	