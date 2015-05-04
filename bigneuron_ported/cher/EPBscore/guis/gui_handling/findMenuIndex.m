function out=findMenuIndex(handle, entry)
% returns the index of a popupmenu that contains the text given by ENTRY
	out=0;
	
	if strcmp(get(handle, 'Style'), 'popupmenu')==0
		disp(['findMenuIndex: called with handle to non-popmenu.  ' get(handle, 'Tag') ' is of style ' get(handle, 'Style')]);
		return
	end
	
	if ~isnumeric(entry)
		val=str2num(entry);
		if isnumeric(val) & length(val)>0
			entry=val;
		end
	end
		
	menuItems=get(handle, 'String');
	
	for i=1:length(menuItems)
		label=menuItems{i};
		val=str2num(label);
		if isnumeric(val) & length(val)>0
			label=val;
			if isnumeric(entry)
				if entry==label
					out=i;
					return
				end
			end
		else
			if ~isnumeric(entry)
				if strcmp(label,entry)
					out=i;
					return
				end
			end
		end
	end
	