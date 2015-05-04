function out=getGuiOfGlobal(globalName)
	out=[];
	[topName, structName, fieldName]=structNameParts(globalName);
	eval(['global ' topName ';']);
	if ~exist(topName, 'var')
		return
	end
	if length(structName)==0								% it is a top level global 
		global globalGUIPairs
		if ~isstruct(globalGUIPairs)
			return
		else
			if ~isfield(globalGUIPairs, topName)
				return
			end
			out=getfield(globalGUIPairs, topName);
		end		
	else								% it is a field of a global
		if ~eval(['isfield(' structName ', ''globalGUIPairs'');'])
			return
		end
		if ~eval(['isfield(' structName '.globalGUIPairs, ''' fieldName ''');'])
			return
		end
		eval(['out=' structName '.globalGUIPairs.' fieldName ';']);
	end
