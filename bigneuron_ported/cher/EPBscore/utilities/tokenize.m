function out=tokenize(line)
	out={};
	line=deblank(line);
	while (length(line)>0)
		[token, line]=getToken(line);
		if (length(token)>0)
			out{length(out)+1}=token;
		end
	end

function [token, remLine]=getToken(line)
	[token, remLine]=strtok(line);
	if length(token)==0
		return
	end
	
	if any(findstr(token,'''')) 
		while (length(remLine)>0) & (token(length(token))~='''') 
			[tok2, remLine]=strtok(remLine);
			remLine=remLine(2:length(remLine));
			token=[token ' ' tok2];
		end
		if token(1)=='''' & token(length(token))==''''
			token=token(2:length(token)-1);
		end
	end	
	
