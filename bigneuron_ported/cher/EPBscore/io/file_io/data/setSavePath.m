function setSavePath
	global state
	
	if ~isempty(state.files.savePath)
		cd(state.files.savePath)
	end
	[fname, pname]=uiputfile('save_path', 'Choose a path');
	if isnumeric(fname)
		return
	end
	status=state.internal.statusString;
	setStatusString('Setting Path...');

	state.files.savePath=pname;
	updateFullFileName(0);
	cd(pname);
	setStatusString(status)	
	disp(['*** SAVE PATH = ' state.files.savePath ' ***']);
	
	
