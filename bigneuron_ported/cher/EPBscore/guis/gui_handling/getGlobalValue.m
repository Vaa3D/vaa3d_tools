function out=getGlobalValue(glo)
	[topName, temp, temp2]=structNameParts(glo);
	eval(['global ' topName ';']);
	eval(['out=' glo ';']);