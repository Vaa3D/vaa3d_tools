function savePositionListAs
	global state

	if ~isempty(state.files.savePath)
		cd(state.files.savePath);
	end

	[fname, pname]=uiputfile('*.pos', 'Choose position list file name');
	if ~isnumeric(fname)
		periods=findstr(fname, '.');
		if any(periods)								
			fname=fname(1:periods(1)-1);
		end		
		
		positionVectors=state.motor.positionVectors;
		save(fullfile(pname, [fname '.pos']), 'positionVectors', '-mat');
		setStatusString('Position list saved')
	end
