function loadCycleModeConfig(position)
	global state gh

	if nargin<1
		position=state.internal.positionToExecute;
	end
	
	state.configName=state.cycle.cycleParts{position};
	state.configPath=state.cycle.cyclePaths{position};

	if isnumeric(state.configName) | length(state.configName)==0
		disp(['loadCycleModeConfig: Cannot open config file ' fullfile(state.configPath, [state.configName '.cfg']) ]);
		return
	end
	
	turnOffMenus;
	turnOffExecuteButtons;
	[flag, fname, pname, ext]=initGUIs([state.configPath '\' state.configName '.cfg']);
	if flag==0
		disp(['loadCycleModeConfig: Cannot open config file ' fullfile(state.configPath, [state.configName '.cfg']) ]);
		turnOnMenus;
		turnOnExecuteButtons;
		return
	end

	state.internal.repeatsTotal=state.cycle.cycleRepeats(position);
	state.internal.secondsCounter=state.cycle.cycleTimeDelay(position);
	state.acq.returnHome=state.cycle.cycleReturnHome(position);
	state.acq.averaging=state.cycle.cycleAveraging(position);
	state.acq.numberOfZSlices=state.cycle.cycleNumberOfZSlices(position);
	state.acq.numberOfFrames=state.cycle.cycleNumberOfFrames(position);
	state.acq.zStepSize=state.cycle.cycleZStepPerSlice(position);

	state.configName=fname;
	state.configPath=pname;

	updateAllGUIVars(gh.mainControls);
	updateHeaderString('state.acq.zStepSize');
	updateHeaderString('state.acq.averaging');
	updateHeaderString('state.acq.returnHome');
	turnOnMenus;
	turnOnExecuteButtons;

	