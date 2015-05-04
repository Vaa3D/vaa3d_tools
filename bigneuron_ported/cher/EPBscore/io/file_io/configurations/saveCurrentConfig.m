function out=saveCurrentConfig()
	global state
	out=0;
	
	[fid, message]=fopen(fullfile(state.configPath, [state.configName '.cfg']), 'wt');
	if fid==-1
		disp(['saveCurrentConfig: Error cannot open output file ' fullfile(state.configPath, [state.configName '.cfg']) ]);
		return
	end
	createConfigFileFast(1, fid, 1);
	fclose(fid);
	out=1;
	cd(state.configPath);
	state.internal.configurationNeedsSaving=0;
	return
	
	