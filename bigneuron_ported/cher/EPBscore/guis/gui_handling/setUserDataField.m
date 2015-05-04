function setUserDataField(handle, varargin)
% sets fields in 'UserData' of a handle. Varargin should be field1, value1 [, field2, value2 ...]
% handles all 'UserData' fields other than 'Gui' which really acts on a global and is set
% through addGuiOfGlobal

	if (length(varargin)>=2)				% make sure there are param, value pairs
        if strcmp(get(handle,'Type'),'uimenu')
            style='uimenu';
        else
            style=get(handle, 'Style');			% get style of handle to adjust behavior 
        end
		ud=get(handle, 'UserData');			
		recheckValid=0;						% flag that determines  whether gui validity needs to be rechecked.
											% this is used to catch invalid entries that might be caused by
											% changing the 'Max' or 'Min' values of a GUI
		for i=1:length(varargin)/2			%loop through pairs
			ud=setfield(ud, varargin{i*2-1}, varargin{i*2});	% add param, value pair to 'UserData'
			if strcmp(style, 'slider') 							
				if any(strcmp(varargin{i*2-1}, {'Min'; 'Max'; 'Value'}))
					if hasGUIBeenUsed(handle) | strcmp(varargin{i*2-1}, 'Value')
						recheckValid=1;							% will need to recheck vaility
					end;
					set(handle, varargin{i*2-1}, varargin{i*2});	% for sliders Max, Min, and Value are handle properties
				end
			elseif strcmp(style, 'edit') 
				if any(strcmp(varargin{i*2-1}, {'String'; 'Value'}))
					recheckValid=1;
					val=varargin{i*2};
					if isnumeric(val)
						val=num2str(val);
					end
					set(handle, 'String', val);
				elseif any(strcmp(varargin{i*2-1}, {'Min'; 'Max'})) & hasGUIBeenUsed(handle)
					recheckValid=1;
				end
			end
		end
		set(handle, 'UserData', ud);
		if recheckValid						% if vailidity needs to be rechecked
			genericCallback(handle);		% check by calling genericCallback routine on the gui
		end
	end
	
	
	