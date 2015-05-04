function varargout=saveCurrentCycleAs
% Saves current cycle under new name
	global state

	if ~isempty(state.cycle.cyclePath)
		cd(state.cycle.cyclePath)
	end

	[fname, pname]=uiputfile('*.cyc', 'Choose cycle name');
	if ~isnumeric(fname)
		periods=findstr(fname, '.');
		if any(periods)								
			fname=fname(1:periods(1)-1);
		end		
		state.cycle.cycleName=fname;
		state.cycle.cyclePath=pname;
		updateGUIbyglobal('state.cycle.cycleName');
		updateGUIbyglobal('state.cycle.cyclePath');
		saveCurrentCycle;
	end
