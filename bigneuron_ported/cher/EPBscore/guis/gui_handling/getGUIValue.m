function out=getGUIValue(handle)
% returns the value in a GUI

	style=get(handle, 'Style');
	if strcmp(style, 'edit')
		out=get(handle, 'String');
		if hasUserDataField(handle, 'Numeric')
			if getUserDataField(handle, 'Numeric')
				out=str2num(out);
			end
		end
	elseif strcmp(style, 'slider')
		out=get(handle, 'Value');
	elseif strcmp(style, 'popupmenu')
		out=get(handle, 'Value'); 
	else
		out=get(handle, 'Value');
	end
