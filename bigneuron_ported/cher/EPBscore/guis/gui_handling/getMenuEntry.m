function out=getMenuEntry(handle, index)
% returns text in an entry of a popupmenu

	out=[];
	
	if strcmp(get(handle, 'Style'), 'popupmenu')==0
		disp(['getMenuEntry: called with handle to non-popmenu.  ' get(handle, 'Tag') ' is of style ' get(handle, 'Style')]);
		return
	end
	
	menuItems=get(handle, 'String');

	if index>=1 & index<=length(menuItems)
		out=menuItems{index};
	end
	
