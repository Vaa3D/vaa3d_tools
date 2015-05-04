function turnOnExecuteButtons
	global gh
	set(gh.mainControls.focusButton, 'enable', 'on')
	set(gh.mainControls.startLoopButton, 'enable', 'on')
	set(gh.mainControls.grabOneButton, 'enable', 'on')
 	set(gh.motorGUI.GRAB, 'enable', 'on')
