function out=openAndLoadCycle
	out=0;

	global state
	if state.internal.cycleChanged==1
		button = questdlg(['Do you want to save changes to ' state.cycle.cycleName '?'],'Save changes?','Yes','No','Cancel','Yes');
		if strcmp(button, 'Cancel')
			disp(['*** LOAD CYCLE CANCELLED ***']);
			return
		elseif strcmp(button, 'Yes')
			disp(['*** SAVING CURRENT CYCLE = ' state.cycle.cyclePath '\' state.cycle.cycleName ' ***']);
			flag=saveCurrentCycle;
			if ~flag
				disp(['loadCycle: Error returned by saveCurrentCycle.  Cycle may not have been saved.']);
				return
			end
		end
	end

	if ~isempty(state.cycle.cyclePath)
		cd(state.cycle.cyclePath)
	end
	
	[fname, pname]=uigetfile('*.cyc', 'Choose cycle file to load');
	if ~isnumeric(fname)
		periods=findstr(fname, '.');
		if any(periods)								
			fname=fname(1:periods(1)-1);
		else
			disp('cycleControls: Error: found file name without extension');
			return
		end		
		loadcycle(fname, pname);
		cd(state.cycle.cyclePath);
		changePositionToExecute(0);
	end
	
