function addGUIOfGlobal(globalName, guiName)
% add a GUI to the list of GUIs associated with a global variable
% this is done by storing GUI NAME in the GLOBAL NAME subfield of the
% corresponding globalGUIPairs structure.  For top level variables (ie no 
% subfields for a structure) the association is stored in the global named
% globalGUIPairs.  For subfields of a structure, the association is stored in a
% new subfield, created on the same level as the variable, called globalGUIPairs.

	[topName, structName, fieldName]=structNameParts(globalName);	% parse the variable name
	if length(topName)==0											% is there a valid name?
		disp(['addGUIOfGlobal: empty global name (guiName = ' guiName ')']);
	end
	eval(['global ' topName ';']);									% get access to the global
	if ~exist(topName, 'var')										% does the variable exist?
		disp(['addGUIOfGlobal: global' globalName 'does not exists (guiName = ' guiName ')']);
		return
	end

	if length(structName)==0			% we are dealing with a top level global 
		global globalGUIPairs
		if ~isstruct(globalGUIPairs)	% if not a structure (or doesn't exist),
			globalGUIPairs=[];			% make it
		end
		if ~isfield(globalGUIPairs, topName)	% if field named topName, make it, and put guiName in it
			globalGUIPairs=setfield(globalGUIPairs, topName, {guiName});
			return
		end
		if ~any(strcmp(getGuiOfGlobal(globalName), {guiName}))	% if field is there, add guiName to cell array
			globalGUIPairs=setfield(globalGUIPairs, topName, [getfield(globalGUIPairs, topName), {guiName}]);	
		end
	else								% we are dealing with the subfield of a global structure. Same logic as above.
		if ~eval(['isfield(' structName ', ''globalGUIPairs'');'])
			eval([structName '.globalGUIPairs=[];']);
		end
		if ~eval(['isfield(' structName '.globalGUIPairs, ''' fieldName ''');'])
			eval([structName '.globalGUIPairs.' fieldName '={''' guiName '''};']);
			return
		end
		if ~any(strcmp(getGuiOfGlobal(globalName), {guiName}))
			eval([structName '.globalGUIPairs.' fieldName '=[' structName '.globalGUIPairs.' fieldName ', {''' guiName '''}];']);	
		end
	end
