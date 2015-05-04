function loadPositionList
	[fname, pname]=uigetfile('*.pos', 'Choose position list file');
	if ~isnumeric(fname)
		global state
		periods=findstr(fname, '.');
		if any(periods)								
			fname=fname(1:periods(1)-1);
		end		
		
		setStatusString('Loading position list...');
		load(fullfile(pname, [fname '.pos']), '-mat');			% load file as MATLAB workspace file
		state.motor.positionVectors=positionVectors;
		if state.motor.position>size(state.motor.positionVectors,1)
			state.motor.position=size(state.motor.positionVectors,1);
			updateGUIbyglobal('state.motor.position');
		end
		setStatusString('Position list loaded...');
		disp('The following position list was loaded:');
		listPositions;
	end
