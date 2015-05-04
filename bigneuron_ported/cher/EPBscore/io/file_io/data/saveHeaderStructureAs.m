function out=saveHeaderStructureAs
	out=0;
	[fname, pname]=uiputfile('header.txt', 'Choose a path');
	if isnumeric(fname)
		return
	end
	setStatusString('Saving Header...');
	global state
	state.files.savePath=pname;
	periods=findstr(fname, '.');
	if ~any(periods)								
		fname=[fname '.txt'];
	end		
	updateFullFileName(0);
	cd(pname);
	fid=fopen(fullfile(pname, fname), 'wt');
	if fid<1
		beep;
		disp(['saveHeaderStructureAs: Error opening ' fullfile(pname, fname) ' for writing.']);
		return 
	end
	createConfigFileFast(2, fid, 0);
	fclose(fid);
	
	setStatusString('')	
	disp(['*** SAVE PATH = ' state.files.savePath ' ***']);
	
	
