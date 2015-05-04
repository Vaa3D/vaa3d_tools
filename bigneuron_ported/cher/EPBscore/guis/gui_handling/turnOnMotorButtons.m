function turnOnMotorButtons
	global gh
	set(gh.motorGUI.setZeroZButton, 'enable', 'on')
	set(gh.motorGUI.setZeroXYButton, 'enable', 'on')
	set(gh.motorGUI.setZeroXYZButton, 'enable', 'on')
	set(gh.motorGUI.shiftXYZButton, 'enable', 'on')
	set(gh.motorGUI.shiftXYButton, 'enable', 'on')
	set(gh.motorGUI.definePosition, 'enable', 'on')
	set(gh.motorGUI.gotoPosition, 'enable', 'on')
	set(gh.motorGUI.readPosition, 'enable', 'on')
