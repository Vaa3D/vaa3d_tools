function setUserDataByGUIName(guiLoc, param, value)
	if ~iscell(guiLoc) | length(guiLoc)==0
		disp(['setUserDataByGUIName: trying to set gui info with no gui ' guiLoc ' ' param ' ' value]);
		return
	end
	if isnumeric(value)
		value=num2str(value);
	else
		value=['''' value ''''];
	end
	for guiCount=1:length(guiLoc)
		[ghName, guiName, temp]= structNameParts(guiLoc{guiCount});
	%	disp([guiLoc{guiCount} ' ' param ' ' value ' ' num2str(guiCount)]);
		eval(['global ' ghName]);
		eval(['setUserDataField(' guiLoc{guiCount} ',''' param ''',' value ');']);
	end

