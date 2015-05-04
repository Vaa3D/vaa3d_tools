function out=saveCurrentCycle;
	out=0;
	global state

	fname=state.cycle.cycleName;
	pname=state.cycle.cyclePath;
	if isnumeric(fname) | length(fname)==0
		[fname, pname]=uiputfile('*.cyc', 'Choose filename and path for cycle');

		if ~isnumeric(fname)
			periods=findstr(fname, '.');
			if any(periods)								
				fname=fname(1:periods(1)-1);
			end		
			state.cycle.cycleName=fname;
			state.cycle.cyclePath=pname;
			updateGUIbyglobal('state.cycle.cycleName');
			updateGUIbyglobal('state.cycle.cyclePath');
		else
			return
		end
	end

	cycle=state.cycle;
	cycle=rmfield(cycle, 'configGlobals');
	cycle=rmfield(cycle, 'globalGUIPairs');
	save(fullfile(pname, [fname '.cyc']), 'cycle', '-mat');
	state.internal.cycleChanged=0;
	out=1;
	

	
