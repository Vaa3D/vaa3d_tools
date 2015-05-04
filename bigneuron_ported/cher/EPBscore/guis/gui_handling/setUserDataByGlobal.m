function setUserDataByGlobal(globalName, param, value)
	guiLoc=getGuiOfGlobal(globalName);
	setUserDataByGUIName(guiLoc, param, value);

