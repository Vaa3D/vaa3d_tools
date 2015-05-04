function updateGUIGlobal(handle)
% update the value of a Global value to match that contained in its GUI
	if hasUserDataField(handle, 'Global')
		gName=getUserDataField(handle, 'Global');
		
		[topName, structName, fieldName]=structNameParts(gName);
		val=getGUIValue(handle);
		if isnumeric(val)
			val=num2str(val);
		else
			val=['''' val ''''];
		end
		eval(['global ' topName]);
		if exist(topName, 'var')
			if length(structName)>0
				eval([structName '=setfield(' structName ',''' fieldName ''',' val ');']);	
			else
				eval([topName '=' val ';']);
			end
			updateGUIbyglobal(gName);	
		else
			Disp(['Error: global ' topName ' not found']);
		end
	end
		
