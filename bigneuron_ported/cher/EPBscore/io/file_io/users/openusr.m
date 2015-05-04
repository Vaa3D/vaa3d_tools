function out=openusr(fileName)
	out=1;
	[fid, message]=fopen(fileName);
	if fid<0
		disp(['openusr: Error opening ' fileName ': ' message]);
		out=1;
		return
	end
	[fileName,permission, machineormat] = fopen(fid);
	fclose(fid);
	
	disp(['*** CURRENT USER SETTINGS FILE = ' fileName ' ***']);
	initGUIs(fileName);
	
	[path,name,ext,ver] = fileparts(fileName);
	
	global state
	state.userSettingsName=name;
	state.userSettingsPath=path;
	saveUserSettingsPath;
	
	state.configName='';
	state.configPath='';
	if length(state.cycle.cycleName)>0
		loadCycleToMemory(state.cycle.cycleName, state.cycle.cyclePath);	
	else
		state.standardMode.standardModeOn=1;
	end

	applyModeCycleAndConfigSettings;
	updateAutoSaveCheckMark;		% BSMOD	
	updateKeepAllSlicesCheckMark; % BSMOD
	updateAutoOverwriteCheckMark;
	
	global gh	% BSMOD added 1/30/1 with lines below

	wins=fieldnames(gh);

	for winCount=1:length(wins)
		winName=wins{winCount};
		if isfield(state.internal, [winName 'Bottom']) & isfield(state.internal, [winName 'Left'])
			pos=get(getfield(getfield(gh, winName), 'figure1'), 'Position');
			pos(1)=getfield(state.internal, [winName 'Left']);
			pos(2)=getfield(state.internal, [winName 'Bottom']);
			set(getfield(getfield(gh, winName), 'figure1'), 'Position', pos);
		end
	end


