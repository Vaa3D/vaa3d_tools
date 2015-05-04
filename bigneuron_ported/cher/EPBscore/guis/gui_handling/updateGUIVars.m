function updateGUIVars(handle)
% updates the value displayed in a GUI to match that of an associated variable
	if ~hasUserDataField(handle, 'Global')
		return
	end
	glo=getUserDataField(handle, 'Global');
	setGUIValue(handle, getGlobalValue(glo));
	%updateHeaderString(glo);
			