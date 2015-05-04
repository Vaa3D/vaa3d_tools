function setGUIValue(handle, val)
% set the value displayed in a GUI to the given value 
    if strcmp(get(handle,'Type'),'uimenu')
            style='uimenu';
    else
           style=get(handle, 'Style');
    end
	switch style 
		case 'edit'
			if hasUserDataField(handle, 'Numeric')
				if getUserDataField(handle, 'Numeric') & ischar(val)
					val=str2num(val);
				end				
			end
			set(handle, 'String', val);
		case 'text'
			set(handle, 'String', val);
		case 'slider'
			if ischar(val)
				val=str2num(val)
			end				
			set(handle, 'Value', val);
		case 'popupmenu'
			if ischar(val)
				val=str2num(val)
			end				
			set(handle, 'Value', val);
		case 'checkbox'
			set(handle, 'Value', val);
		case 'radiobutton'
			set(handle, 'Value', val);
        case 'togglebutton'
			set(handle, 'Value', val);
        case 'listbox'
			set(handle, 'Value', val);  
        case 'uimenu'
		otherwise
			disp(['setGUIValue: Style not implemented ' style]);
	end
