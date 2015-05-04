function out=trimZerosFromString(str)
	out=str;
	
	zs=findstr(str, 0);
	if any(zs)
		out=str(1:zs(1)-1);
	end
	
	