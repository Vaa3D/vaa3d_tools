function seeGUI(GUI)
% makes a gui visible.  expects string with name of gui
	[topName, s, f]=structNameParts(GUI);
	eval(['global ' topName]);
	eval(['set(' GUI ', ''Visible'', ''on'')']);
	
