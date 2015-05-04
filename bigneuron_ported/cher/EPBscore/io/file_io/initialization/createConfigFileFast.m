function createConfigFile(bitFlags, fid, outputFlag)
	global configGlobals
	if isstruct(configGlobals)
		fNames=fieldnames(configGlobals);
		for i=1:length(fNames)
			recurseCreateConfigFile(fNames{i}, bitFlags, '', fid, outputFlag);
		end
	end
	recurseCreateConfigFile('state', bitFlags, '', fid, outputFlag);
	
	
function recurseCreateConfigFile(startingName, bitFlags, pad, fid, outputFlag)
	if length(startingName)==0
		return
	end
	
	[topName, structName, fieldName]=structNameParts(startingName);
	eval(['global ' topName]);

	if eval(['iscell(' startingName ');'])
		return
	end			
	if length(fieldName)==0
		fieldName=topName;
	end
	if eval(['~isstruct(' startingName ');'])
		if any(bitand(getGlobalConfigStatus(startingName),bitFlags)) | bitFlags==0			% if 0, output everything for ini file
			val=[];
			eval(['val=' startingName ';']);
			if isnumeric(val)
				if length(val)>1
					val=['[' num2str(val) ']'];
				else
					val=num2str(val);
				end
			else
				val=['''' val ''''];
			end
			if outputFlag==0
				fprintf(fid, '%s\n', [pad fieldName]);
			else
				fprintf(fid, '%s=%s\n', [pad fieldName], val);
			end				
		end
	else
		if ~exist(topName, 'var')
			return 
		end
		if length(fieldName)==0
			fieldName=topName;
		end
		fprintf(fid, [pad 'structure ' fieldName '\n']);
		fNames=[];
		eval(['fNames=fieldnames(' startingName ');']);
		for i=1:length(fNames)
			if ~any(strcmp(fNames{i}, {'configGlobals', 'globalGUIPairs'}))
				recurseCreateConfigFile([startingName '.' fNames{i}], bitFlags, [pad '   '], fid, outputFlag);
			end
		end
		fprintf(fid, [pad 'endstructure\n']);
	end
	