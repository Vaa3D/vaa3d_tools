function out=hasGUIBeenUsed(handle)
	out=hasUserDataField(handle, 'LastValid');
	