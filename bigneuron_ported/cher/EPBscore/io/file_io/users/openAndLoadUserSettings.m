function out=openAndLoadUserSettings
% Allows user to select a settings file (*.ini) from disk and loads it
% Author: Bernardo Sabatini
	out=0;

	global state
	status=state.internal.statusString;
	setStatusString('Loading user settings...');

	[fname, pname]=uigetfile('*.usr', 'Choose user settings file to load');
	if ~isnumeric(fname)
		periods=findstr(fname, '.');
		if any(periods)								
			fname=fname(1:periods(1)-1);
		else
			disp('openAndLoadUserSettings: Error: found file name without extension');
			setStatusString('Can''t open file...');
			return
		end		
		openusr(fullfile(pname, [fname '.usr']));
		cd(state.userSettingsPath);
		
		
% 		global gh
% 	
% 		wins=fieldnames(gh);
% 	
% 		for winCount=1:length(wins)
% 			winName=wins{winCount};
% 			if isfield(state.internal, [winName 'Bottom']) & isfield(state.internal, [winName 'Left'])
% 				pos=get(getfield(getfield(gh, winName), 'figure1'), 'Position');
% 				pos(1)=getfield(state.internal, [winName 'Left']);
% 				pos(2)=getfield(state.internal, [winName 'Bottom']);
% 				set(getfield(getfield(gh, winName), 'figure1'), 'Position', pos);
% 			end
% 		end
	end
	setStatusString(status);
