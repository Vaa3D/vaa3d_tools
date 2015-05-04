function opencyc(fileName)
	global state
	if state.internal.cycleChanged==1
		button = questdlg(['Do you want to save changes to ' state.cycle.cycleName '?'],'Save changes?','Yes','No','Cancel','Yes');
		if strcmp(button, 'Cancel')
			disp(['*** LOAD CYCLE CANCELLED ***']);
			return
		elseif strcmp(button, 'Yes')
			disp(['*** SAVING CURRENT CYCLE ' state.cycle.cyclePath '\' state.cycle.cycleName ' ***']);
			flag=saveCurrentConfig;
			if ~flag
				return
			end
		end
	end

	[pname, fname, ext]=fileparts(fileName);
	loadCycle(fname,pname);
	disp(['*** CYCLE ' state.cycle.cyclePath '\' state.cycle.cycleName ' LOADED ***']);
	