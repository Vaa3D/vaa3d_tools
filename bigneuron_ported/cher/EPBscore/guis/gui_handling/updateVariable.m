function updateVariable(handle)
% automatically updates a global variable when a GUI value has been changed
	if (hasUserDataField(handle, 'varName')==0)
		return
	end
	fieldName=getUserDataField(handle, 'varName');
	global gloVar
	gloVar=setfield(gloVar, fieldName, get(handle, 'String'));
	
