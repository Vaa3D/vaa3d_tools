import ij.*;
import ij.process.*;
import ij.gui.*;
import ij.plugin.*;
import ij.plugin.frame.*;
import ij.io.*;
import ij.plugin.filter.PlugInFilter;
import java.awt.*;
import java.lang.*;
import java.io.*;
import java.util.*;
//2014.01.16 brl modifying to write .v3draw from virtual stack without having to read whole file into memory

//brl corrected a bug in rgb writes 2013.12.05

public class raw_writerBRL implements PlugInFilter {

	private String formatkey = "raw_image_stack_by_hpeng";
	private ImagePlus imp;
	
	public int setup(String arg, ImagePlus imp)
	{
		this.imp = imp;
		return DOES_ALL+NO_CHANGES;
	}
	
	public void run(ImageProcessor ip) {
		SaveDialog sd = new SaveDialog("Save V3D's Raw Image...",imp.getTitle(),".v3draw");
		String directory = sd.getDirectory();
		String fileName = sd.getFileName();
		IJ.showStatus("Saving: " + directory + fileName);
		
		try{
		// brl changed to use random access file class
// FileOutputStream out = new FileOutputStream(directory+fileName);
			RandomAccessFile out = new RandomAccessFile(directory+fileName, "rw");
			//format key
			out.write(formatkey.getBytes());
			
			//endianness, big-endian is default
			out.write("B".getBytes());
			
			int imtype = imp.getType();
			int[] dim = new int[5];
			dim = imp.getDimensions();
			if (dim[4]!=1)
				throw new Exception("Currently the plugin does not support 5d hyperstack. Please check your data.");
			int[] sz = new int[4];
			sz[0] = dim[0];
			sz[1] = dim[1];
			sz[2] = dim[3];
			sz[3] = dim[2];// 
			int unitSize;
			
			//IJ.showMessage("imtype="+imtype);



			//unitSize & image size depends on the type of ImagePlus
			switch (imtype) {   
			case ImagePlus.COLOR_RGB:
				unitSize = 1;     // BRL corrected this case to be compatible with 32-bit RGB images like the vaa3d example file ex_Repo_hb9_eve.tiff
					sz[3] = 3;    // Previous code just fell through without correcting the number of channels.
					break;
			case ImagePlus.GRAY8:
				unitSize = 1;
				break;
			case ImagePlus.GRAY16:
				unitSize = 2;
				break;
			case ImagePlus.GRAY32:
				unitSize = 4;
				break;
			default:
				unitSize = imp.getBitDepth()/8;
			}

			
			
			
			
			out.write(int2byte(unitSize,2));
			for (int i=0;i<4;i++) 
				out.write(int2byte(sz[i],4));
			
	
			
			int w = sz[0];
	        int h = sz[1];
	        int nChannel = sz[3];
	        int bitdepth = imp.getBitDepth();

		
			int layerOffset = w*h;
			long colorOffset = layerOffset*(long)sz[2];
			long totalUnit = colorOffset*sz[3];
			long totalsize=totalUnit*unitSize;
			int bytesPerPixel=imp.getBytesPerPixel();
			
			
			//	IJ.showMessage("file pointer before writing"+out.getFilePointer());
	
			//	IJ.showMessage("Image Type imtype= "+imtype);

			
			
//			IJ.showMessage("w="+w+" h="+h+" s="+sz[2]+" c="+nChannel);
//			IJ.showMessage("imtype="+imtype);
//			IJ.showMessage("Bit Depth="+bitdepth);
//			IJ.showMessage("unit size="+unitSize);
//			IJ.showMessage("Bytes_Per_Pixel="+bytesPerPixel);
//			IJ.showMessage("colorOffset="+colorOffset);
			
//			IJ.showMessage("totalBytes="+sz[3]);
			
			ImageStack stack = imp.getStack();



			
			// previously this plugin put the whole image into memory before writing				
///		 ByteArray64 img = new ByteArray64(totalUnit*unitSize);
			// but now I can do the same thing (?!) with the RandomAccessFile
							//			IJ.showMessage("before switch imtype?  "+imtype);

			switch (imtype) {
			case ImagePlus.COLOR_RGB:
				byte[] r = new byte[layerOffset];
				byte[] g = new byte[layerOffset];
				byte[] b = new byte[layerOffset];
				long currentRedPointer = 43;
				long currentGreenPointer = 43+colorOffset;
				long currentBluePointer = 43+2*colorOffset;
				for (int layer=1;layer<=sz[2];layer++)
				{
					ColorProcessor cp = (ColorProcessor)stack.getProcessor(layer);
					cp.getRGB(r,g,b);
					
					    out.seek(currentRedPointer);
						out.write(r); 
						currentRedPointer = out.getFilePointer();
						
						out.seek(currentGreenPointer);
						out.write(g);
						currentGreenPointer = out.getFilePointer();
						
						out.seek(currentBluePointer);
						out.write(b);
						currentBluePointer = out.getFilePointer();
						
					
					
					
				}
				r = null;
				g = null;
				b = null;
				break;
				
			case ImagePlus.GRAY8:
						//	IJ.showMessage("gray8 ?  "+imtype+"blah");
				byte[] imtmp = new byte[layerOffset];
				long[] currentPointers = new long[sz[3]];
				int[]  channellist = new int[sz[3]];
				channellist[0]=sz[3];
				for (int cNumber=1;cNumber<sz[3];cNumber++)
				{ 
				channellist[cNumber]=cNumber;
				currentPointers[cNumber-1] = 43+(long)(cNumber-1)*colorOffset;
				}
				for (int layer=1;layer<=sz[2];layer++)
				{
					for (int channelIndex=0;channelIndex<sz[3];channelIndex++)
					{
						out.seek(currentPointers[channelIndex]);
						imtmp = (byte[])stack.getPixels((layer-1)*sz[3]+channellist[channelIndex]);
						out.write(imtmp); 
						currentPointers[channelIndex] = out.getFilePointer();							
					}
				}
				break;
				
			case ImagePlus.GRAY16:
				//short[] im16 = new short[layerOffset];
				byte[] im16asbytes = new byte[2*layerOffset];
				for (int layer=1;layer<=sz[2];layer++)
				{
					for (int colorChannel=0;colorChannel<sz[3];colorChannel++)
					{
						
					im16asbytes = ShortToByte_Twiddle_Method((short[])stack.getPixels((layer-1)*sz[3]+colorChannel+1));
					out.write(im16asbytes);
						// im16 = (short[])stack.getPixels((layer-1)*sz[3]+colorChannel+1);
								//out.writeShort(im16);
						/*for (int i=0;i<layerOffset;i++)
						{
							byte[] tmp = int2byte(im16[i],2);
							out.write( tmp[0]);
							out.write( tmp[1]);
							
					
						}*/
					}
				}
				//im16 = null;
				im16asbytes=null;
				break;/*
			case ImagePlus.GRAY32:
				float[] im32 = new float[layerOffset];
				for (int layer=1;layer<=sz[2];layer++)
				{
					for (int colorChannel=0;colorChannel<sz[3];colorChannel++)
					{
						im32 = (float[])stack.getPixels((layer-1)*sz[3]+colorChannel+1);
						for (int i=0;i<layerOffset;i++)
						{
							byte[] tmp = int2byte(Float.floatToIntBits(im32[i]),4);
							out.write(4*colorChannel*colorOffset+(layer-1)*layerOffset*4+4*i, tmp[0]);
							out.write(4*colorChannel*colorOffset+(layer-1)*layerOffset*4+4*i+1, tmp[1]);
							out.write(4*colorChannel*colorOffset+(layer-1)*layerOffset*4+4*i+2, tmp[2]);
							out.write(4*colorChannel*colorOffset+(layer-1)*layerOffset*4+4*i+3, tmp[3]);
						}
					}
				}
				im32 = null;
				break;
				*/
			default:
				throw new Exception("Image type not supported by this plugin.");
			}
			
			//img.write(out);
			
			
			out.close();
			IJ.showMessage("script is finished");

			
		} catch ( Exception e ) {
	          IJ.error("Error:" + e.toString());
	          return;
	        }
	}
	
	byte[] int2byte(int num, int len)
	{
		byte[] by = new byte[len];
		for (int i=len-1;i>=0;i--)
		{
			by[i] = (byte)(num & 0xFF);
			num = num >> 8;
		}
		return(by);
	}
	
	byte [] ShortToByte_Twiddle_Method(short [] input) //brl borrowed from stackexchange 
{
  int short_index, byte_index;
  int iterations = input.length;

  byte [] buffer = new byte[input.length * 2];

  short_index = byte_index = 0;

  for(/*NOP*/; short_index != iterations; /*NOP*/)
  {
    buffer[byte_index]     = (byte) ((input[short_index] & 0xFF00) >> 8);
    buffer[byte_index + 1] = (byte) (input[short_index] & 0x00FF);

    ++short_index; byte_index += 2;
  }

  return buffer;
}
	
}


