Vaa3d writer plugin for Fiji
Brian Long
2014.02.19  

To install: 
copy Vaa3d_writer.jar into the plugins directory of Fiji and restart Fiji.

Now you will find an entry "V3Draw" under the menu:
File> Save as> 

This plugin is compatible with virtual stacks and minimizes RAM requirements by writing stacks one slice at a time. 


In order to build the .class file from .java source, you need to include the path to the imageJ jar files.

for example, while in the FijiWriter directory, enter

javac ./Vaa3d_Writer.java -cp "<YOUR_FIJI_DIR>/jars/*"

then create the appropriate .jar file using the included configuration

jar cf Vaa3d_Writer.jar Vaa3d_Writer.* plugins.config