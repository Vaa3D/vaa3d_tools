function updateAllGUIVars(handle)
% udpates all GUIs in handle
	GUINames=fieldnames(handle);
	for i=1:length(GUINames)
		gui=getfield(handle, GUINames{i});
		if (hasUserDataField(gui, 'Global'))
			updateGUIVars(gui);
		end
	end
	