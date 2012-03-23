macro "lsm_tif_converter"
{
	print("\\Clear");
	prefix = getDirectory("Choose a Directory");
	name = getFileList(prefix);
	print(prefix);
	fullname = Array.copy(name);
	File.makeDirectory(prefix + "converted");
	
	for (i=0;i<name.length;i++)
	{
		fullname[i] = prefix + name[i];
		print(fullname[i]);
		print(name[i]);
	}
	
	run("Bio-Formats Macro Extensions");
	for (i=0;i<name.length;i++)
	{
		Ext.openImagePlus(fullname[i]);
		getDimensions(dummy, dummy, nChannel, dummy, dummy);
		run("Split Channels");
		for (c=1;c<=nChannel;c++)
		{
			selectImage(1);
			print(getTitle());
			saveAs("tiff", prefix + "/converted/" + name[i] + "_c_" + c + '.tif');
			close();
		}
	}
	Ext.close();
	
}