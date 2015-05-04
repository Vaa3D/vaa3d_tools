function out=getListEntry(handle, index)
% returns text in an entry of a popupmenu

	out=[];
	
	menuItems=get(handle, 'String');

	if index>=1 & index<=length(menuItems)
		out=menuItems{index};
	end
	
