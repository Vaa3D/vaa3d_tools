function retVal=getUserDataField(handle, field)
% returns value of field in 'UserData' or [] if that field does not exist
	if (hasUserDataField(handle, field))
		retVal=getfield(get(handle, 'UserData'), field);
	else
		retVal=[];
	end
	