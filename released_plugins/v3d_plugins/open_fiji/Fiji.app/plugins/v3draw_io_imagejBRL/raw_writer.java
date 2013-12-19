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
//brl corrected a bug in rgb writes 2013.12.05
public class raw_writer implements PlugInFilter {

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
			FileOutputStream out = new FileOutputStream(directory+fileName);
			
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

		//	imtype=1;
		//	IJ.showMessage("imtype="+imtype);
		
			int layerOffset = w*h;
			long colorOffset = layerOffset*(long)sz[2];
			long totalUnit = colorOffset*sz[3];
			long totalsize=totalUnit*unitSize;
			int bytesPerPixel=imp.getBytesPerPixel();
			
//			IJ.showMessage("w="+w+" h="+h+" s="+sz[2]+" c="+nChannel);
//			IJ.showMessage("imtype="+imtype);
//			IJ.showMessage("Bit Depth="+bitdepth);
//			IJ.showMessage("unit size="+unitSize);
//			IJ.showMessage("Bytes_Per_Pixel="+bytesPerPixel);
//			IJ.showMessage("colorOffset="+colorOffset);
			
//			IJ.showMessage("totalBytes="+sz[3]);
			
			ImageStack stack = imp.getStack();
			ByteArray64 img = new ByteArray64(totalUnit*unitSize);
			
			switch (imtype) {
			case ImagePlus.COLOR_RGB:
				byte[] r = new byte[layerOffset];
				byte[] g = new byte[layerOffset];
				byte[] b = new byte[layerOffset];
				for (int layer=1;layer<=sz[2];layer++)
				{
					ColorProcessor cp = (ColorProcessor)stack.getProcessor(layer);
					cp.getRGB(r,g,b);
					for (int i=0;i<layerOffset;i++)
					{
						img.set((layer-1)*layerOffset+i, r[i]);
						img.set(colorOffset+(layer-1)*layerOffset+i, g[i]);
						img.set(2*colorOffset+(layer-1)*layerOffset+i, b[i]);
					}
				}
				r = null;
				g = null;
				b = null;
				break;
			case ImagePlus.GRAY8:
				byte[] imtmp = new byte[layerOffset];
				for (int layer=1;layer<=sz[2];layer++)
				{
					for (int colorChannel=0;colorChannel<sz[3];colorChannel++)
					{
						imtmp = (byte[])stack.getPixels((layer-1)*sz[3]+colorChannel+1);
						for (int i=0;i<layerOffset;i++)
							img.set(colorChannel*colorOffset+(layer-1)*layerOffset+i, imtmp[i]);
					}
				}
				break;
			case ImagePlus.GRAY16:
				short[] im16 = new short[layerOffset];
				for (int layer=1;layer<=sz[2];layer++)
				{
					for (int colorChannel=0;colorChannel<sz[3];colorChannel++)
					{
						im16 = (short[])stack.getPixels((layer-1)*sz[3]+colorChannel+1);
						for (int i=0;i<layerOffset;i++)
						{
							byte[] tmp = int2byte(im16[i],2);
							img.set(2*colorChannel*colorOffset+(layer-1)*layerOffset*2+2*i, tmp[0]);
							img.set(2*colorChannel*colorOffset+(layer-1)*layerOffset*2+2*i+1, tmp[1]);
						}
					}
				}
				im16 = null;
				break;
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
							img.set(4*colorChannel*colorOffset+(layer-1)*layerOffset*4+4*i, tmp[0]);
							img.set(4*colorChannel*colorOffset+(layer-1)*layerOffset*4+4*i+1, tmp[1]);
							img.set(4*colorChannel*colorOffset+(layer-1)*layerOffset*4+4*i+2, tmp[2]);
							img.set(4*colorChannel*colorOffset+(layer-1)*layerOffset*4+4*i+3, tmp[3]);
						}
					}
				}
				im32 = null;
				break;
			default:
				throw new Exception("Image type not supported by this plugin.");
			}
			
			img.write(out);
			
			
			out.close();
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
}

class ByteArray64 {

    private final long CHUNK_SIZE = 1024*1024*1024; //1GiB

    long size;
    byte [][] data;

    public ByteArray64( long size ) {
        this.size = size;
        if( size == 0 ) {
            data = null;
        } else {
            int chunks = (int)(size/CHUNK_SIZE);
            int remainder = (int)(size - ((long)chunks)*CHUNK_SIZE);
            data = new byte[chunks+(remainder==0?0:1)][];
            for( int idx=chunks; --idx>=0; ) {
                data[idx] = new byte[(int)CHUNK_SIZE];
            }
            if( remainder != 0 ) {
                data[chunks] = new byte[remainder];
            }
        }
    }
	/**  brl mod for debugging indexing bug 2013.12.2
	 */
    public byte get( long index ) {
        if( index<0 || index>=size ) {
            throw new IndexOutOfBoundsException("GET Error attempting to access data element "+index+".  Array is "+size+" elements long.");
        }
        int chunk = (int)(index/CHUNK_SIZE);
        int offset = (int)(index - (((long)chunk)*CHUNK_SIZE));
        return data[chunk][offset];
    }
    public void set( long index, byte b ) {
        if( index<0 || index>=size ) {
            throw new IndexOutOfBoundsException("SET Error attempting to access data element "+index+".  Array is "+size+" elements long.");
        }
        int chunk = (int)(index/CHUNK_SIZE);
        int offset = (int)(index - (((long)chunk)*CHUNK_SIZE));
        data[chunk][offset] = b;
    }
    /**
     * Simulates a single read which fills the entire array via several smaller reads.
     * 
     * @param fileInputStream
     * @throws IOException
     */
    public void read( FileInputStream fileInputStream ) throws IOException {
        if( size == 0 ) {
            return;
        }
        for( int idx=0; idx<data.length; idx++ ) {
            if( fileInputStream.read( data[idx] ) != data[idx].length ) {
                throw new IOException("short read.");
            }
        }
    }
    public void write( FileOutputStream fileOutputStream ) throws IOException {
        if( size == 0 ) {
            return;
        }
        for( int idx=0; idx<data.length; idx++ ) 
            fileOutputStream.write( data[idx] );
    }
    public long size() {
        return size;
    }
}

