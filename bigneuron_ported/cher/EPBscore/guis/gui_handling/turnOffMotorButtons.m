function turnOffMotorButtons
	global gh
	set(gh.motorGUI.setZeroZButton, 'enable', 'off')
	set(gh.motorGUI.setZeroXYButton, 'enable', 'off')
	set(gh.motorGUI.setZeroXYZButton, 'enable', 'off')
	set(gh.motorGUI.shiftXYZButton, 'enable', 'off')
	set(gh.motorGUI.shiftXYButton, 'enable', 'off')
	set(gh.motorGUI.definePosition, 'enable', 'off')
	set(gh.motorGUI.gotoPosition, 'enable', 'off')
	set(gh.motorGUI.readPosition, 'enable', 'off')

