function out=openAndLoadConfiguration
% Allows user to select a configuration from disk and loads it
% Author: Bernardo Sabatini
	out=0;

	global state

	status=state.internal.statusString;
	setStatusString('Loading Configuration...');
	if state.internal.configurationNeedsSaving==1
		button = questdlg(['Do you want to save changes to ''' state.configName '''?'],'Save changes?','Yes','No','Cancel','Yes');
		if strcmp(button, 'Cancel')
			disp(['*** LOAD CYCLE CANCELLED ***']);
			setStatusString('Cancelled');
			return
		elseif strcmp(button, 'Yes')
			disp(['*** SAVING CURRENT CONFIGURATION = ' state.configPath '\' state.configName ' ***']);
			flag=saveCurrentConfig;
			if ~flag
				disp(['openAndLoadConfiguration: Error returned by saveCurrentCycle.  Cycle may not have been saved.']);
				setStatusString('Error saving file');
				return
			end
			state.internal.configurationNeedsSaving=0;
		end
	end
	
	if ~isempty(state.configPath)
		cd(state.configPath)
	end
	[fname, pname]=uigetfile('*.cfg', 'Choose configuration to load');
	if ~isnumeric(fname)
		periods=findstr(fname, '.');
		if any(periods)								
			fname=fname(1:periods(1)-1);
		else
			disp('openAndLoadConfiguration: Error: found file name without extension');
			setStatusString('Can''t open file');
			return
		end	
		state.standardMode.configName=fname;
		state.standardMode.configPath=pname;
		turnOffMenus;
		turnOffExecuteButtons;
		loadStandardModeConfig;
		turnOnMenus;
		turnOnExecuteButtons;
	end
	setStatusString(status);
