function loadStandardModeConfig()
	global state;
	out=0;
	
	state.configName=state.standardMode.configName;
	state.configPath=state.standardMode.configPath;
	configSelected=1;
	
	if isnumeric(state.configName) | length(state.configName)==0
		configSelected=0;
	else	
		[flag, fname, pname, ext]=initGUIs([state.configPath '\' state.configName '.cfg']);
		if flag==0
			configSelected=0;
		end
	end
	
	if configSelected
		setStatusString('Config loaded');
		state.configName=fname;
		state.configPath=pname;
	else
		setStatusString('Using config in memory');
		disp('loadStandardModeConfig: No configuration selected.  Using parameters currently in memory');
		state.configName='Default';
		state.configPath='';
	end

	state.acq.numberOfFrames=state.standardMode.numberOfFrames;
	updateGUIbyglobal('state.acq.numberOfFrames');

	state.internal.secondsCounter=state.standardMode.repeatPeriod;
	updateGUIbyglobal('state.internal.secondsCounter');

	state.acq.numberOfZSlices=state.standardMode.numberOfZSlices;
	updateGUIbyglobal('state.acq.numberOfZSlices');

	state.acq.zStepSize=state.standardMode.zStepPerSlice;
	updateHeaderString('state.acq.zStepSize');

	state.acq.averaging=state.standardMode.averaging;
	updateHeaderString('state.acq.averaging');
		
	state.acq.returnHome=state.standardMode.returnHome;
	updateHeaderString('state.acq.returnHome');

	applyChannelSettings;
	setStatusString('');

	
