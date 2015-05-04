function setGlobalConfigStatus(globalName, configStatus)
	[topName, structName, fieldName]=structNameParts(globalName);
	if ~isnumeric(configStatus)
		val=str2num(configStatus);
		if isnumeric(val)
			configStatus=val;
		else
			disp(['setGlobalConfigStatus: number as config status. Got ' configStatus ]);
			return
		end
	end
	
	eval(['global ' topName ';']);
	if length(topName)==0
		disp(['setGlobalConfigStatus: empty global name (configStatus = ' configStatus ')']);
	end
	if ~exist(topName, 'var')
		disp(['setGlobalConfigStatus: global' globalName 'does not exists (configStatus = ' configStatus ')']);
		return
	end

	if length(structName)==0			% we are dealing with a top level global 
		global configGlobals
		if ~isstruct(configGlobals)
			configGlobals=[];
		end
		configGlobals=setfield(configGlobals, topName, configStatus);
	else								% we are dealing with the field of a global
		if ~eval(['isfield(' structName ', ''configGlobals'');'])
			eval([structName '.configGlobals=[];']);
		end
		eval([structName '.configGlobals.' fieldName '=configStatus;']);
	end
