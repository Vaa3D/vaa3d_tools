function out=padLength(st, len)
	out=st;
	if length(st)<len
		out(length(st)+1:len)=0;
	elseif length(st)>len
		out=st(1:len);
	end
	return
