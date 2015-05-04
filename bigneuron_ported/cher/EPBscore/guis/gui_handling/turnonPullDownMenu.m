function turnonPullDownMenu(menu, label)
	children=get(menu, 'Children');
	for counter=1:length(children)
		if strcmp(get(children(counter), 'Label'), label)
			set(children(counter), 'Enable', 'on');
			return
		end
	end
