function resumeLoop
	global state gh
	updateGUIbyglobal('state.internal.secondsCounter');
		
	state.internal.positionJustExecuted=state.internal.positionToExecute;
	state.internal.repeatsDone=state.internal.repeatsDone+1;
	updateGUIbyglobal('state.internal.repeatsDone');
	set(gh.mainControls.focusButton, 'Visible', 'On');
		
	if state.standardMode.standardModeOn==0
		moveStageNeeded=0;
	
		if state.cycle.cycleDX(state.internal.positionJustExecuted)~=0
			moveStageNeeded=1;
			state.motor.absXPosition=state.motor.absXPosition+state.cycle.cycleDX(state.internal.positionJustExecuted);
		end
			 
		if state.cycle.cycleDY(state.internal.positionJustExecuted)~=0
			moveStageNeeded=1;
			state.motor.absYPosition=state.motor.absYPosition+state.cycle.cycleDY(state.internal.positionJustExecuted);
		end
	
		if state.cycle.cycleDZ(state.internal.positionJustExecuted)~=0
			moveStageNeeded=1;
			state.motor.absZPosition=state.motor.absZPosition+state.cycle.cycleDZ(state.internal.positionJustExecuted);
		end

		if moveStageNeeded==1
			setStatusString('Moving Stage...');
			setMotorPosition;
			updateRelativeMotorPosition;
			setStatusString('');
		end
	else
	end
	state.internal.lastTimeDelay=state.cycle.cycleTimeDelay(state.internal.positionToExecute);
	mainLoop;
