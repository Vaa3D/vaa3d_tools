function out=mainLoop
	out=1;
	global state gh
	
	setStatusString('Looping...');

	if state.internal.abort
		state.internal.abort=0;
		out=0;
		state.internal.firstTimeThroughLoop=1;
		return
	end
	
	change=0;
	if ~state.internal.cyclePaused
		if ~state.standardMode.standardModeOn
			if state.internal.repeatsDone>=state.internal.repeatsTotal
				state.internal.repeatsDone=0;
				state.internal.positionToExecute=state.internal.positionToExecute+1;
				change=1;
			end
	
			if state.internal.positionToExecute>state.cycle.length
				state.internal.positionToExecute=1;
				state.internal.repeatsDone=0;
				change=1;
				if state.cycle.returnHomeAtCycleEnd & ~state.internal.firstTimeThroughLoop & state.motor.motorOn
					setStatusString('Moving to cycle home...');
					MP285SetVelocity(state.motor.velocityFast);
					if length(state.internal.cycleInitialMotorPosition)~=3
						setStatusString('Cycle home not defined');
						disp('mainLoop: Cannot return to cycle home.  Cycle home not defined');
					else
						state.motor.absXPosition=state.internal.cycleInitialMotorPosition(1);
						state.motor.absYPosition=state.internal.cycleInitialMotorPosition(2);
						state.motor.absZPosition=state.internal.cycleInitialMotorPosition(3);
						setMotorPosition;
						MP285SetVelocity(state.motor.velocitySlow);
						updateRelativeMotorPosition;
					end
				end
			end

			if change==1
				if state.cycle.length > 1
					setStatusString('Loading new config...');
					changePositionToExecute;
				end
			end
			
			if state.internal.repeatsDone==0 & state.cycle.cycleStartingPosition(state.internal.positionToExecute)>0
				gotoPosition(state.cycle.cycleStartingPosition(state.internal.positionToExecute));
			end
		else
			state.internal.lastTimeDelay=state.standardMode.repeatPeriod;
			if state.internal.firstTimeThroughLoop
				state.internal.repeatsDone=0;
				updateGUIbyglobal('state.internal.repeatsDone');
			end
		end
	
		if state.acq.numberOfZSlices > 1 & state.motor.motorOn	% & state.acq.returnHome
			state.internal.initialMotorPosition=updateMotorPosition;
		else
			state.internal.initialMotorPosition=[];
		end				

		if state.internal.firstTimeThroughLoop | (state.internal.lastTimeDelay<=0 & ~state.standardMode.standardModeOn)
			state.internal.lastTimeDelay=state.cycle.cycleTimeDelay(state.internal.positionToExecute);
		end
	
		state.internal.looping=1;

		updateGUIbyglobal('state.internal.repeatsDone');
		updateGUIbyglobal('state.internal.positionToExecute');
		updateGUIbyglobal('state.internal.frameCounter');
		updateGUIbyglobal('state.internal.zSliceCounter');
	
		if state.internal.firstTimeThroughLoop==0
			state.internal.secondsCounter=floor(state.internal.lastTimeDelay-etime(clock,state.internal.triggerTime));
		else
			state.internal.secondsCounter=state.internal.lastTimeDelay;
		end

		updateGUIbyglobal('state.internal.secondsCounter');
	else
		state.internal.cyclePaused=0;
		state.internal.looping=1;
		if ~state.standardMode.standardModeOn
			setStatusString('Loading config...');
			changePositionToExecute;
		end
		
		if state.acq.numberOfZSlices > 1 & state.motor.motorOn	% & state.acq.returnHome
			state.internal.initialMotorPosition=updateMotorPosition;
		else
			state.internal.initialMotorPosition=[];
		end				
	end

	startZoom;
	if state.init.autoReadPMTOffsets
		startPMTOffsets;
	end

	% load daq engine % here get dacq ready for trigger
	if state.internal.firstTimeThroughLoop==0
		setStatusString('Counting down...');

		if etime(clock,state.internal.triggerTime)>(state.internal.lastTimeDelay-1)
			setStatusString('DELAY TOO SHORT!');
			beep;
		end

		while etime(clock,state.internal.triggerTime) <(state.internal.lastTimeDelay-1)
			if state.internal.cyclePaused
				return
			end
			if state.internal.abort==1
				state.internal.abort=0;
				state.internal.firstTimeThroughLoop=1;
				state.internal.looping=0;
				out=0;
				return
			end
			old=etime(clock,state.internal.triggerTime);
%			updateMotorPosition;
			while floor(etime(clock,state.internal.triggerTime))<old
				pause(0.01);
				if state.internal.cyclePaused
					return
				end
				if state.internal.abort==1
					state.internal.abort=0;
					state.internal.firstTimeThroughLoop=1;
					out=0;
					return
				end
			end
			state.internal.secondsCounter=round(state.internal.lastTimeDelay-etime(clock,state.internal.triggerTime));
			updateGUIbyglobal('state.internal.secondsCounter');
			pause(0.01);
		end

		state.internal.secondsCounter=0;
		if strcmp(get(gh.mainControls.focusButton, 'String'), 'FOCUS')
			set(gh.mainControls.focusButton, 'Visible', 'Off');
		else
			setStatusString('STOP FOCUS!');
			disp('mainLooop:  Interrupting loop because focus was running at trigger time');
			state.internal.looping=0;
			return
		end
		if state.internal.lastTimeDelay-etime(clock,state.internal.triggerTime)-state.internal.timingDelay>0
			pause(state.internal.lastTimeDelay-etime(clock,state.internal.triggerTime)-state.internal.timingDelay) % 0.05 is 
		end
	else
		if state.internal.abort==1
			state.internal.abort=0;	
			state.internal.firstTimeThroughLoop=1;
			out=0;	
			set(gh.mainControls.focusButton, 'Visible', 'On');
			return
		end
		set(gh.mainControls.focusButton, 'Visible', 'Off');
		if state.cycle.returnHomeAtCycleEnd & state.internal.positionToExecute==1 ...
				& state.internal.repeatsDone==0 & ~state.standardMode.standardModeOn & state.motor.motorOn
			setStatusString('Defining cycle home...');
			state.internal.cycleInitialMotorPosition=updateMotorPosition;
		end
			
		state.internal.firstTimeThroughLoop=0;

	end

	if ~state.standardMode.standardModeOn
		state.internal.lastTimeDelay=state.cycle.cycleTimeDelay(state.internal.positionToExecute);
	end

	%state.internal.stopAcq = 0; Updated to match berbnardo's code
	if state.internal.abort	% Updaeted via BS on 1/16/02
		 state.internal.abort=0;
		 state.internal.firstTimeThroughLoop=1;
		 out=0;
		 set(gh.mainControls.focusButton, 'Visible','On');
		 return
	 end
	 
	setStatusString('Acquiring...');
	
	state.internal.stripeCounter=0;
	state.internal.forceFirst=1;
	resetCounters;
	
	startGrab;
	openShutter;
	diotrigger;

	disp(['Executed ''' state.configName ''' at ' clockToString(clock)]);
	disp(['   Seconds since last acquisition: ' num2str(etime(clock,state.internal.triggerTime))]);
	
	
	
	
