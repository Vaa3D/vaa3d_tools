function turnOnMenus

	global gh
	turnonPullDownMenu(gh.mainControls.Settings, 'Edit Cycle...');
	turnonPullDownMenu(gh.mainControls.Settings, 'Edit Configuration...');
	turnonPullDownMenu(gh.mainControls.Settings, 'Standard Cycle Mode');
	turnonPullDownMenu(gh.mainControls.Settings, 'Channels...');
	turnonPullDownMenu(gh.mainControls.File, 'Load User Settings...');
	turnonPullDownMenu(gh.mainControls.File, 'Load Configuration...');
	turnonPullDownMenu(gh.mainControls.File, 'Load Cycle...');
	turnonPullDownMenu(gh.mainControls.Settings, 'Get PMT Offsets...');
	
	set(get(gh.standardModeGUI.figure1, 'children'), 'Enable', 'On');
	set(get(gh.cc.figure1, 'children'), 'Enable', 'On');
	set(gh.mainControls.cyclePosition, 'Enable', 'On');
	set(gh.mainControls.positionToExecuteSlider, 'Enable', 'On');
