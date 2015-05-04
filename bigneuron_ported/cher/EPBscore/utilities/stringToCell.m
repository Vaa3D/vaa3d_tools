function out=stringToCell(inString)
	out=[];
	if ~ischar(inString)
		return
	end
	
	if inString(1)~='{'
		if inString(1)=='''' & inString(end)==''''
			inString=inString(2:end-1);
		end
		out={inString};
		return
	end
	quotes=findstr(inString, '''');
	if length(quotes)>2
		for counter=1:2:length(quotes)
			out=[out, {inString(quotes(counter)+1:quotes(counter+1)-1)}];
		end
	end
			
	
	
		
