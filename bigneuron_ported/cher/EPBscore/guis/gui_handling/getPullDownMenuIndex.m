function out=getPullDownMenuIndex(menu, label)
	children=get(menu, 'Children');
	for counter=1:length(children)
		if strcmp(get(children(counter), 'Label'), label)
			out=counter;
			return
		end
	end
	out=0;
