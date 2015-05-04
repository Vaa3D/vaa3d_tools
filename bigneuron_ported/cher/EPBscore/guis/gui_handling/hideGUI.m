function hideGUI(GUI)
	[topName, s, f]=structNameParts(GUI);
	eval(['global ' topName]);
	eval(['set(' GUI ', ''Visible'', ''off'')']);
