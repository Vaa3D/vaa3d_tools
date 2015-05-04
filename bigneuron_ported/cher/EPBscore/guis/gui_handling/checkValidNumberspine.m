function out=checkValidNumberspine(handle)
% function to check if entry in GUI is valid
	out=1;
	style=get(handle, 'Style');
	min=[];
	max=[];
	valField=[];
	if strcmp(style, 'slider')
		val=get(handle, 'Value');
		valField='Value';
		min=get(handle, 'Min');
		max=get(handle, 'Max');
	elseif strcmp(style, 'edit')
		if hasUserDataField(handle, 'Numeric')
			if ~getUserDataField(handle, 'Numeric')
				return
			end
			val=str2num(get(handle, 'String'));
		else
			val=str2num(get(handle, 'String'));
			if length(val)~=1
				return
			end
		end
		
		valField='String';
		min=getUserDataField(handle, 'Min');
		max=getUserDataField(handle, 'Max');
	elseif strcmp(style, 'popupmenu')
		val=get(handle, 'Value');
		valField='Value';
		min=1;
		max=length(get(handle, 'String'));
	elseif strcmp(style, 'checkbox') | strcmp(style, 'radiobutton') |strcmp(style, 'togglebutton') |strcmp(style, 'listbox')
		val=get(handle, 'Value');
	elseif strcmp(style, 'text')
		return
	else
		disp(['checkValidNUmber: Unimplemented style :' style]);
	end
	
	if isnumeric(val) & length(val)==1 
		if length(min)==1
			if (val < min)
				val=min;
			end
		end
		if length(max)==1
			if (val > max)
				val=max;
			end
		end
		if strcmp(style, 'edit')
			val=num2str(val);
		end
		if length(valField)>0
			set(handle, valField, val);
		end
		setUserDataField(handle, 'LastValid', val);	
	elseif strcmp(style, 'edit')
		if hasUserDataField(handle, 'LastValid')
			set(handle, 'String', getUserDataField(handle, 'LastValid'));
		else		
			set(handle, 'String', '0');
			out=0;
		end
	elseif strcmp(style, 'slider')
		if hasUserDataField(handle, 'LastValid')
			set(handle, 'Value', getUserDataField(handle, 'LastValid'));
		else		
			out=0;
		end
	end
	
	
