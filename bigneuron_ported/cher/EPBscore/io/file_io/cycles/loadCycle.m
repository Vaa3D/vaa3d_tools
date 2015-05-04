function loadCycle(fname, pname);
% Load a experiment cycle from disk given a name and a path
% Cycles are saved as MATLAB workspace files because it is an easy way to 
% save cell array contents without having to write a parser.

	out=1;
	cycle=[];
	global state
	
	fid=fopen(fullfile(pname, [fname '.cyc']), 'r');		% try to open file to ensure it exists
	if fid==-1												% error on opening, abort
		disp(['loadCycle: Could not open file ' fullfile(pname, [fname '.cyc']) '.  Loading of cycle aborted.']);
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
	state.internal.position=1;
	
	updateCycleLengthSlider(0);
	loadCurrentCyclePosition;
	
	state.cycle.cycleName=fname;
	state.cycle.cyclePath=pname;
	
	updateGUIbyglobal('state.internal.position');
	updateGUIbyglobal('state.cycle.length');
	updateGUIbyglobal('state.cycle.cycleName');
	updateGUIbyglobal('state.cycle.cyclePath');
	updateGUIbyglobal('state.cycle.returnHomeAtCycleEnd');
	state.internal.cycleChanged=0;
	
	disp(['*** CURRENT CYCLE = ' fullfile(pname, [fname '.cyc']) ' ***']);
	setStatusString('');
