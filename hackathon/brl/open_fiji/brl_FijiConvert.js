//loader = IJ.getClassLoader();
//raw_readerX = loader.getResource("raw_reader.class");
//importClass("Applications.Fiji.app.plugins.3draw_io_imagejBRL.raw_reader.class")
//IJ.getClassLoader().getResource("raw_reader.class")

name = getArgument();
argarray = name.split(":"); 

if (name=="") 
{
IJ.error("javascript called with no argument!");
}
//print(name); 
imp = ImagePlus();
var types = new Array();
         types[ImagePlus.GRAY8] = "8-bit";  
         types[ImagePlus.GRAY16]= "16-bit";  
         types[ImagePlus.GRAY32] = "32-bit";  
         types[ImagePlus.COLOR_256] = "8-bit color";
         types[ImagePlus.COLOR_RGB] =  "RGB";  
         
inname = argarray[0];
outname= argarray[1];


print("input path name "+inname);
print("output path name "+outname);

inextension = inname.substring(inname.lastIndexOf("."));
print("input extension is "+inextension)
if ((inextension==".v3draw") || (inextension==".raw"))
{
print("reading .v3draw or .raw file with raw reader");
//raw_readerX.run("open="+inname);
//imp=Opener(inname);
IJ.run(imp, "raw reader", "open="+inname);
imp = IJ.getImage()
}
else 
{
imp = IJ.openImage(inname);
//imp=Opener(inname);
//imp=Opener.openUsingBioFormats(argarray[0]);
}

outextension = outname.substring(outname.lastIndexOf("."));
if (outextension==outname) 
{
outextension=""; // if there's no "." , set the extension to nothing
print("no output file extension, appending '.v3draw' and saving")
}
else 
{
print("output extension is " +outextension);
}

//print( "image type: " +imp.getType());
//print("N Channels: " +imp.NChannels)
//if ((imp.getType()==4)||(inextension==".v3draw")||(imp.NChannels>0))
//{
//print(" making composite for avi file");
//IJ.run(imp, "Make Composite", "");
//}

if (outextension==".avi") 
{  //add some code here to allow user to set compression and framerate?
//print(" making composite for avi file");
//IJ.run(imp, "Make Composite", "");
//
print(" making composite for avi file");
IJ.run(imp, "Make Composite", ""); //this generates the headless error because this plugin wants a window to show up. 
								   // however, it doesn't seem to kill the script
//imp = IJ.getImage();
compression="JPEG";
framerate="7";
IJ.run(imp, "AVI... ", "compression=" +compression +" frame=" +framerate +" save=" +outname);
}
else if (outextension==".v3draw") 
{
//raw_writer
IJ.run(imp, "raw writer", "save="+outname);
imp.close();
}
else if (outextension=="")
{
outname=outname.concat(".v3draw");
print("final output path name "+outname);
IJ.run(imp, "raw writer", "save="+outname);
imp.close();
}

