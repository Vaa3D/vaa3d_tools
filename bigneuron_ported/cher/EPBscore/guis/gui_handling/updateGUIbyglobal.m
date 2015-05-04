function updateGUIbyglobal(globalName)
% given the name of a global variable, find and update the GUI that contains it
	guiLoc=getGuiOfGlobal(globalName);
	if length(guiLoc)==0
		% update header for those variables that do not have GUIs
	%	updateHeaderString(globalName);		
	else	
		updateGUIByName(guiLoc);	% update the GUI
	end
