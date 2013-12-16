import ij.*;
import ij.process.*;
import ij.gui.*;
import java.awt.*;
import ij.plugin.*;
import ij.plugin.frame.*;
import java.io.*;
import ij.io.*;
import java.awt.image.*;
import java.io.*;

public class raw_reader implements PlugIn {

    private String formatkey = "raw_image_stack_by_hpeng";
    
    public void run(String arg) {
        OpenDialog od = new OpenDialog("Open V3D's Raw Image...", arg);
        String fileName = od.getFileName();
        if (fileName==null)       return;
        String directory = od.getDirectory();
        IJ.showStatus("Opening: " + directory + fileName);
       
        // Read in the header values...
        try {
          File f = new File(directory + fileName);
          FileInputStream fid = new FileInputStream(f);
          int lenkey = formatkey.length();
          long fileSize = f.length();     
         
          //read the format key
          if (fileSize<lenkey+2+4*4+1) // datatype has 2 bytes, and sz has 4*4 bytes and endian flag has 1 byte.
              throw new Exception("The size of your input file is too small and is not correct, -- it is too small to contain the legal header.");
          byte[] by = new byte[lenkey];
          long nread = fid.read(by);
          String keyread = new String(by);
          if (nread!=lenkey)
              throw new Exception("File unrecognized or corrupted file.");
          if (!keyread.equals(formatkey))
              throw new Exception("Unrecognized file format.");
          by = null;
         
         
          //read the endianness
          by = new byte[1];
          fid.read(by);
          if (by[0]!='B' && by[0]!='L')
              throw new Exception("This program only supports big- or little- endian but not other format. Check your endian.");
        
          boolean isBig = (by[0]=='B');
          by = null;
         
         
          //read the data type info
          by = new byte[2];
          fid.read(by);
          short dcode = (short)bytes2int(by,isBig);
         
          int datatype;
          switch (dcode){
          case 1:
              datatype = 1;
              break;
          case 2:
              datatype = 2;
              break;
          case 4:
              datatype = 4;
              break;
          default:
              throw new Exception("Unrecognized datatype code"+dcode+". The file is incorrect or this code is not supported in this version");
          }
          int unitSize = datatype;
          by = null;
          
          //read the data size info (the data size is stored in either 2-byte or 4-byte space)       
          long[] sz = new long[4];
          long totalUnit = 1;
          
          //first assume this is a 2-byte file
          by = new byte[2];
          for (int i=0;i<4;i++)
          {
              fid.read(by);
              sz[i] = bytes2int(by,isBig);
              totalUnit *= sz[i];
          }         
          by = null;
          
          if ((totalUnit*unitSize+4*2+2+1+lenkey) != fileSize)
          {
        	  //see if this is a 4-byte file
        	  if (isBig)  {
        		sz[0] = sz[0]*64+sz[1];
        		sz[1] = sz[2]*64+sz[3];
        	  }
        	  else {
        		sz[0] = sz[1]*64+sz[0];
        		sz[1] = sz[3]*64+sz[2];
        	  }
        	  by = new byte[4];
        	  for (int i=2;i<4;i++)
        	  {
        		  fid.read(by);
        		  sz[i] = bytes2int(by,isBig);
        	  }
        	  totalUnit = 1;
        	  for (int i=0;i<4;i++)
        		  totalUnit *= sz[i];
        	  if ((totalUnit*unitSize+4*4+2+1+lenkey) != fileSize)
        		  throw new Exception("The input file has a size different from what specified in the header. Exit.");
        	}
         
          
          
          //read the pixel info
          
          
          ByteArray64 img = new ByteArray64(totalUnit*unitSize);
          img.read(fid);
         
          
          //construct img into an array of ImageStacks, the length of array equals number of color channels.
          
          int w = (int)sz[0];
          int h = (int)sz[1];
          int nChannel = (int)sz[3];
          
          int layerOffset = w*h;
          long colorOffset = layerOffset*sz[2];
          
          //IJ.showMessage("w="+w+" h="+h+" s="+sz[2]+" c="+nChannel);
          ImagePlus[] imps = new ImagePlus[nChannel];
          
          for (int colorChannel=0;colorChannel<nChannel;colorChannel++)
          {
        	  ImageStack imStack = new ImageStack(w,h);
          
	          switch (unitSize) {
	          case 1:
        		  for (long layer=0;layer<sz[2];layer++)
        		  {
        			  ByteProcessor cF = new ByteProcessor(w,h);
            		  byte[] imtmp = new byte[layerOffset];
            		  for (int i=0;i<layerOffset;i++)
        				 imtmp[i] = img.get(colorChannel*colorOffset+layer*layerOffset+i);
            		  cF.setPixels(imtmp);
            		  imStack.addSlice(null,cF);
        		  }
	        	  break;
	          case 2:
	    		  byte[] bytmp = new byte[2];
	    		  for (long layer=0;layer<sz[2];layer++)
	    		  {
	    			 short[] im16 = new short[layerOffset];
	    			 ShortProcessor cF16 = new ShortProcessor(w,h);
	    			 for (int i=0;i<layerOffset;i++){
	    				 bytmp[0] = img.get(colorChannel*colorOffset*2+layer*layerOffset*2+i*2);
	    				 bytmp[1] = img.get(colorChannel*colorOffset*2+layer*layerOffset*2+i*2+1);
	    				 im16[i] = (short)bytes2int(bytmp,isBig);
	    			 }
	    			 cF16.setPixels(im16);
	    			 imStack.addSlice(null,cF16);
	    		  }
	    		  bytmp = null;
	        	  break;
	          case 4:
	    		  bytmp = new byte[4];
	    		  for (long layer=0;layer<sz[2];layer++)
	    		  {
	    			  float[] im32 = new float[layerOffset];
	    			  FloatProcessor cF32 = new FloatProcessor(w,h);
	    			  for (int i=0;i<layerOffset;i++){
	    				 bytmp[0] = img.get(colorChannel*colorOffset*4+layer*layerOffset*4+i*4);
	    				 bytmp[1] = img.get(colorChannel*colorOffset*4+layer*layerOffset*4+i*4+1);
	    				 bytmp[2] = img.get(colorChannel*colorOffset*4+layer*layerOffset*4+i*4+2);
	    				 bytmp[3] = img.get(colorChannel*colorOffset*4+layer*layerOffset*4+i*4+3);
	    				 im32[i] = Float.intBitsToFloat(bytes2int(bytmp,isBig));
	    			  }
	    			 cF32.setPixels(im32);
	    			 imStack.addSlice(null,cF32);
	    		  }
	    		  bytmp = null;
	    		  break;
	          default:
	        	  throw new Exception("format not supported by raw");
	          }
	          imps[colorChannel] = new ImagePlus(null,imStack);
          }
          
          if (sz[3]>1){
	          ImagePlus imPlus = RGBStackMerge.mergeChannels(imps,true);
	          imPlus.show();
          }
          else imps[0].show();
          
          fid.close();
        } catch ( Exception e ) {
          IJ.error("Error:" + e.toString());
          return;
        }
    }
   
    public static final int bytes2int(byte[] b,boolean isBig)
    {
        int retVal = 0;
        if (!isBig)
            for (int i=b.length-1;i>=0;i--) {
            	retVal = (retVal<<8) + (b[i] & 0xff);
            }
        else
            for (int i=0;i<b.length;i++) {
                retVal = (retVal<<8) + (b[i] & 0xff);
            }
       
        return retVal;
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
    public byte get( long index ) {
        if( index<0 || index>=size ) {
            throw new IndexOutOfBoundsException("Error attempting to access data element "+index+".  Array is "+size+" elements long.");
        }
        int chunk = (int)(index/CHUNK_SIZE);
        int offset = (int)(index - (((long)chunk)*CHUNK_SIZE));
        return data[chunk][offset];
    }
    public void set( long index, byte b ) {
        if( index<0 || index>=size ) {
            throw new IndexOutOfBoundsException("Error attempting to access data element "+index+".  Array is "+size+" elements long.");
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