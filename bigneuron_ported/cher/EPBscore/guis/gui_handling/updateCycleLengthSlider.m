function updateCycleLengthSlider(handle)


	global state gh;
	
	updateGUIbyglobal('state.internal.position');
	if state.internal.position>state.cycle.length
		state.internal.position=state.cycle.length;
		updateGUIbyglobal('state.cycle.position');
		loadCurrentCyclePosition;
	end
	if state.internal.positionToExecute>state.cycle.length
		state.internal.positionToExecute=1;
		updateGUIbyglobal('state.cycle.positionToExecute');
		changePositionToExecute;
	end
	if state.cycle.length<=1
		set(gh.cc.cyclePositionSlider, 'Max', 2);
		set(gh.cc.cyclePositionSlider, 'Visible', 'off');
		set(gh.mainControls.positionToExecuteSlider, 'Max', 2);
		set(gh.mainControls.positionToExecuteSlider, 'Visible', 'off');
	else
		set(gh.cc.cyclePositionSlider, 'Max', state.cycle.length);
		set(gh.cc.cyclePositionSlider, 'SliderStep', [1/(state.cycle.length-1), 1/(state.cycle.length-1)]);
		set(gh.cc.cyclePositionSlider, 'Visible', 'on');
		set(gh.mainControls.positionToExecuteSlider, 'Max', state.cycle.length);
		set(gh.mainControls.positionToExecuteSlider, 'SliderStep', [1/(state.cycle.length-1), 1/(state.cycle.length-1)]);
		set(gh.mainControls.positionToExecuteSlider, 'Visible', 'on');
	end	
	state.internal.cycleChanged=1;
	newCycleLength;
	
