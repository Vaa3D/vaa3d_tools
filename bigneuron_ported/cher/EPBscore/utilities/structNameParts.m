function [outTopName, outStructName, outFieldName] = structNameParts(s)
	s=deblank(s);
	periods=findstr(s, '.');
	if length(periods)>0
		outTopName=s(1:periods(1)-1);
		outStructName=s(1:periods(length(periods))-1);
		outFieldName=s(periods(length(periods))+1:length(s));
	else
		outTopName=s;
		outStructName=[];
		outFieldName=[];
	end