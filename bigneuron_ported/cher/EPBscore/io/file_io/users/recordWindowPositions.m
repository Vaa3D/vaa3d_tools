function recordWindowPositions
	global state gh
	
	wins=fieldnames(gh);
	
	for winCount=1:length(wins)
		winName=wins{winCount};
		pos=get(getfield(getfield(gh, winName), 'figure1'), 'Position');
		state.internal=setfield(state.internal, [winName 'Bottom'], pos(2));
		state.internal=setfield(state.internal, [winName 'Left'], pos(1));
	end
		
		