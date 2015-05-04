function varargout =saveCurrentConfigAs
	global state

	if ~isempty(state.configPath)
		cd(state.configPath)
	end
	
	[fname, pname]=uiputfile('*.cfg', 'Choose cycle name');

	if ~isnumeric(fname)
		setStatusString('Saving config...');
	
		periods=findstr(fname, '.');
		if any(periods)								
			fname=fname(1:periods(1)-1);
		end		
		state.configName=fname;
		state.configPath=pname;
		updateGUIbyglobal('state.configName');
		saveCurrentConfig;
		setStatusString('');
		state.internal.configurationNeedsSaving=0;
	else
		setStatusString('Cannot open file');
	end
