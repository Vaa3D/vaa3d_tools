function out=openini(fileName)
	out=1;

	[fid, message]=fopen(fileName);
	if fid<0
		disp(['openini: Error opening ' fileName ': ' message]);
		out=1;
		return
	end
	[fileName,permission, machineormat] = fopen(fid);
	fclose(fid);
	
	disp(['*** CURRENT INI FILE = ' fileName ' ***']);
	initGUIs(fileName);
	
	[path,name,ext,ver] = fileparts(fileName);
	
	global state;
	state.iniName=name;
	state.iniPath=path;
	
