function updateHeaderString(globalName)
	global state

	flag=getGlobalConfigStatus(globalName);
	if ~bitand(flag, 2)
		return
	end
	
	pos=findstr(state.headerString, [globalName '=']);
	
	val=eval(globalName);
	if ~isnumeric(val) & ~ischar(val)
		val
		disp(['updateHeaderString: unknown type for ' globalName]);
		val='0';
	elseif isnumeric(val)
		if length(val)~=1
			val=0;
		end
        val=num2str(val);
	else
		val=['''' val ''''];
	end

	if length(pos)==0
		state.headerString=[state.headerString globalName '=' val 13];
	else
		cr=findstr(state.headerString, 13);
		index=find(cr>pos);
		next=cr(index(1));
		if length(next)==0
			state.headerString=[state.headerString(1:pos-1) globalName '=' val 13];
		else
			state.headerString=[state.headerString(1:pos-1) globalName '=' val state.headerString(next:end)];
		end
	end
	
