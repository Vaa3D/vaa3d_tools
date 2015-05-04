function updateGUILinks(handle)
	if hasUserDataField(handle, 'Links')
		val=getGUIValue(handle);
		if isnumeric(val)
			val=num2str(val);
		else
			val=['''' val ''''];
		end
		allLinks=getUserDataField(handle, 'Links');
		commas=[0 findstr(allLinks, ',') length(allLinks)+1];
		for counter=1:length(commas)-1
			link = allLinks(commas(counter)+1:commas(counter+1)-1);
			colon=findstr(link, ':');
			if length(colon)~=1
				disp('error in link field. Wrong number of colons');
			else
				gui=link(1:colon-1);
				param=link(colon+1:length(link));
				[ghName, guiName, temp]=structNameParts(gui);
				eval(['global ' ghName ';']);
				if exist(ghName, 'var')
					eval(['setUserDataField(' gui ',''' param ''',' val ');']);
				else
					disp(['global guihandle ' ghName 'does not exist']);
				end
			end
		end
	end
	