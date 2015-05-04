function turnOffMenus

	global gh
	turnoffPullDownMenu(gh.mainControls.Settings, 'Edit Cycle...');
	turnoffPullDownMenu(gh.mainControls.Settings, 'Edit Configuration...');
	turnoffPullDownMenu(gh.mainControls.Settings, 'Standard Cycle Mode');
	turnoffPullDownMenu(gh.mainControls.Settings, 'Channels...');
	turnoffPullDownMenu(gh.mainControls.Settings, 'Get PMT Offsets...');
	turnoffPullDownMenu(gh.mainControls.File, 'Load User Settings...');
	turnoffPullDownMenu(gh.mainControls.File, 'Load Configuration...');
	turnoffPullDownMenu(gh.mainControls.File, 'Load Cycle...');
	
	set(get(gh.standardModeGUI.figure1, 'children'), 'Enable', 'Off');
	set(get(gh.cc.figure1, 'children'), 'Enable', 'Off');
	set(gh.mainControls.cyclePosition, 'Enable', 'Off');
	set(gh.mainControls.positionToExecuteSlider, 'Enable', 'Off');

