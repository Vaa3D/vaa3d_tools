function out=opencfg(fileName)
% opens and processes a CONFIG (*.CFG) FILE
	out=0;
	global state
	if state.internal.configurationChanged==1
		button = questdlg(['Do you want to save changes to ' state.configName '?'],'Save changes?','Yes','No','Cancel','Yes');
		if strcmp(button, 'Cancel')
			disp(['*** LOAD CONFIGURATION CANCELLED ***']);
			return
		elseif strcmp(button, 'Yes')
			disp(['*** SAVING CURRENT CONFIGURATION = ' state.configPath '\' state.configName ' ***']);
			flag=saveCurrentConfig;
			if ~flag
				disp(['opencfg: Error returned by saveCurrentConfig.  Configuration may not have been saved.']);
				return
			end
		end
	end
	
	[fid, message]=fopen(fileName);
	if fid==-1
		disp(['opencfg: Error opening .cfg file: ' message]);
		return
	end
	[fileName,permission, machineormat] = fopen(fid);
	fclose(fid);
	
	disp(['*** CURRENT CONFIGURATION = ' fileName ' ***']);
	flag=initGUIs(fileName);
	if flag==0
		disp(['opencfg: Error opening .cfg file. Error returned by initGUIs']);
		return
	end
	
	[path,name,ext,ver] = fileparts(fileName);
	
	state.configName=name;
	state.configPath=path;	
	state.internal.modifiedConfig=0;
	updateGUIbyglobal('state.configName');
	out=1;
	
