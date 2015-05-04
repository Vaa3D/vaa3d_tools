function s=zeroPadNum2Str(n)
	s=num2str(n);
	if length(s)==1
		s=['00' s];
	elseif length(s)==2
		s=['0' s];
	end
	return