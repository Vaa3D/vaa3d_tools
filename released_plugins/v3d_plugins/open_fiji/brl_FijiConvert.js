// 2013.12  BRL script to be executed in command line calls to Fiji.

name = getArgument(); // the single argument has to be split by a ":" into input and output files
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
IJ.run(imp, "raw reader", "open="+inname);     
imp = IJ.getImage()
}
else 
{
imp = IJ.openImage(inname);
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

if (outextension==".avi") 
{  //add some code here to allow user to set compression and framerate?
//print(" making composite for avi file");
//IJ.run(imp, "Make Composite", "");
//
print(" making composite for avi file");
IJ.run(imp, "Make Composite", ""); //this generates the headless error because this plugin wants a window to show up. 
								   // however, it doesn't seem to kill the script
compression="JPEG";
framerate="7";
IJ.run(imp, "AVI... ", "compression=" +compression +" frame=" +framerate +" save=" +outname);
}
else if (outextension==".v3draw") 
{
//raw_writer
IJ.run(imp, "raw writerBRL", "save="+outname);
imp.close();
}
else if (outextension=="")
{
outname=outname.concat(".v3draw");
print("final output path name "+outname);
IJ.run(imp, "raw writerBRL", "save="+outname);
imp.close();
}

