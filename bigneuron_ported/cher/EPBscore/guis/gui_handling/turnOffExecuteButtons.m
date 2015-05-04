function turnOffExecuteButtons
	global gh
	set(gh.mainControls.focusButton, 'enable', 'off')
	set(gh.mainControls.startLoopButton, 'enable', 'off')
	set(gh.mainControls.grabOneButton, 'enable', 'off')
	set(gh.motorGUI.GRAB, 'enable', 'off')
