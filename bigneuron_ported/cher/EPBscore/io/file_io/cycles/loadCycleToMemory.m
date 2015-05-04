function loadCycleToMemory(fname, pname);
% Load a experiment cycle from disk given a name and a path
% Cycles are saved as MATLAB workspace files because it is an easy way to 
% save cell array contents without having to write a parser.
% Cycle settings are not actually put into effect.  Use applyModeCycleAndConfigSettings
% to have settings take effect

	cycle=[];
	global state
	
	fid=fopen(fullfile(pname, [fname '.cyc']), 'r');		% try to open file to ensure it exists
	if fid==-1												% error on opening, abort
		disp(['loadCycleToMemory: Could not open file ' fullfile(pname, [fname '.cyc']) '.  Loading of cycle aborted.']);
		out=0;
		return
	end
	fclose(fid);											% no error, file exists, close it
	setStatusString('Loading cycle...');
	load(fullfile(pname, [fname '.cyc']), '-mat');			% load file as MATLAB workspace file
	
	if isfield(cycle, 'configGlobals')
		cycle=rmfield(cycle, 'configGlobals');
	end
	if isfield(cycle, 'globalGUIPairs')
		cycle=rmfield(cycle, 'globalGUIPairs');
	end

	configFields=fieldnames(cycle);
	for i=1:length(configFields)
		eval(['state.cycle.' configFields{i} '=cycle.' configFields{i} ';']);
	end
	
	state.cycle.cycleName=fname;
	state.cycle.cyclePath=pname;
	updateCycleLengthSlider(0);
	state.internal.position=1;
	loadCurrentCyclePosition;
	
	state.internal.cycleChanged=0;

	disp(['*** LOADED CYCLE = ' fullfile(pname, [fname '.cyc']) ' ***']);
	setStatusString('Cycle Loaded.');
