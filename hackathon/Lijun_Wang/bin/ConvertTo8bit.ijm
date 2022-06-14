macro "Data Converter 8-bit"
{
	dir_saving = getDirectory("choose a directory to save");
	dir_processing = getDirectory("choose a directory to process");
	list = getFileList(dir_processing);//Get a List of Images
	for(i = 0; i < list.length; i++) 
	{
		
                                open(list[i]);
		run("8-bit");
		title = getTitle();
		stitle = substring(title, 0, lengthOf(title)-4);
		//run("Bio-Formats Windowless Importer", "open="+dir_processing + list[i]);//Open each image without an option window
		run("V3Draw...", "save="+dir_saving+stitle+".v3draw");
		close();
		close(list[i]);
	}
}