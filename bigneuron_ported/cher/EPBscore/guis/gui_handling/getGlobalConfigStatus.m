function out=getGlobalConfigStatus(globalName)
	out=0;
	[topName, structName, fieldName]=structNameParts(globalName);

	eval(['global ' topName ';']);
	if length(topName)==0
		disp('getGlobalConfigStatus: empty global name');
	end
	if ~exist(topName, 'var')
		disp(['getGlobalConfigStatus: global' globalName 'does not exists']);
		return
	end

	if length(structName)==0			% we are dealing with a top level global 
		global configGlobals
		if ~isstruct(configGlobals)
			return
		end
		out=getfield(configGlobals, topName);
		if length(out)>1
			out=out(1);
		end
		return
	else								% we are dealing with the field of a global
		if eval(['isfield(' structName ', ''configGlobals'');'])
			if eval(['isfield(' structName '.configGlobals, ''' fieldName ''');'])
				out=eval([structName '.configGlobals.' fieldName ';']);
				if length(out)>1
					out=out(1);
				end
			end
		end
		return
	end
