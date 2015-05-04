function setGlobalConfigStatus(globalName, flag)
	[topName, structName, fieldName]=structNameParts(globalName);
	if ~isnumeric(flag)
		val=str2num(flag);
		if isnumeric(val)
			flag=val;
		else
			disp(['addOrRemoveConfigGlobal: expecting 0 or 1 as flag. Got ' flag ]);
			return
		end
	end
	
	eval(['global ' topName ';']);
	if length(topName)==0
		disp(['addOrRemoveConfigGlobal: empty global name (flag = ' flag ')']);
	end
	if ~exist(topName, 'var')
		disp(['addOrRemoveConfigGlobal: global' globalName 'does not exists (flag = ' flag ')']);
		return
	end

	if length(structName)==0			% we are dealing with a top level global 
		global configGlobals
		if ~isstruct(configGlobals)
			configGlobals=[];
		end
		configGlobals=setfield(configGlobals, topName, flag);
	else								% we are dealing with the field of a global
		if ~eval(['isfield(' structName ', ''configGlobals'');'])
			eval([structName '.configGlobals=[];']);
		end
		eval([structName '.configGlobals.' fieldName '=' flag '};']);
	end
